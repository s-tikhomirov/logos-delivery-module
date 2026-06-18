// Step 16 Approach A feasibility probe.
//
// Models delivery_module storeQuery: the module owner thread (Qt main) blocks in
// callApiRetValue on a semaphore without pumping the event loop, while liblogosdelivery
// invokes the eligibility provider on a worker thread. The provider would call
// LogosAPI::callModule, which uses logos::runOnOwnerThread (BlockingQueuedConnection).
//
// runOnOwnerThread below mirrors logos-cpp-sdk cpp/logos_thread_marshal.h (installed
// SDK headers omit that file; behavior matches SDK callModule marshaling).

#include <atomic>
#include <chrono>
#include <semaphore>
#include <thread>
#include <type_traits>

#include <QCoreApplication>
#include <QMetaObject>
#include <QObject>
#include <QThread>

#include <logos_test.h>

namespace logos_probe {

template <typename Fn>
auto runOnOwnerThread(QObject* obj, Fn&& fn) -> decltype(fn())
{
    using Ret = decltype(fn());
    static_assert(!std::is_reference_v<Ret>,
                  "runOnOwnerThread does not support reference return types");
    if (QThread::currentThread() == obj->thread()) {
        return fn();
    }
    if constexpr (std::is_void_v<Ret>) {
        QMetaObject::invokeMethod(obj, [&]() { fn(); }, Qt::BlockingQueuedConnection);
        return;
    } else {
        Ret ret{};
        QMetaObject::invokeMethod(obj, [&]() { ret = fn(); }, Qt::BlockingQueuedConnection);
        return ret;
    }
}

} // namespace logos_probe

LOGOS_TEST(approachA_callModuleFromWorker_whileOwnerBlocksOnSem_withoutEventLoop) {
    QObject owner;

    LOGOS_ASSERT_EQ(QThread::currentThread(), owner.thread());

    std::atomic<bool> marshaled{false};
    std::binary_semaphore workerDone{0};

    std::thread worker([&] {
        logos_probe::runOnOwnerThread(&owner, [&] { marshaled = true; });
        workerDone.release();
    });

    const bool finishedWithoutPump =
        workerDone.try_acquire_for(std::chrono::milliseconds(400));

    LOGOS_ASSERT_FALSE(finishedWithoutPump);
    LOGOS_ASSERT_FALSE(marshaled.load());

    while (!workerDone.try_acquire_for(std::chrono::milliseconds(50))) {
        QCoreApplication::processEvents();
    }

    worker.join();
    LOGOS_ASSERT_TRUE(marshaled.load());
}

LOGOS_TEST(approachA_callModuleFromWorker_whileOwnerPumpsEvents_succeeds) {
    QObject owner;

    std::atomic<bool> marshaled{false};
    std::binary_semaphore workerDone{0};

    std::thread worker([&] {
        logos_probe::runOnOwnerThread(&owner, [&] { marshaled = true; });
        workerDone.release();
    });

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline) {
        QCoreApplication::processEvents();
        if (workerDone.try_acquire()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    worker.join();
    LOGOS_ASSERT_TRUE(marshaled.load());
}
