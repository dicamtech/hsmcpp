#include "hsm/AsyncHsm.hpp"
#include "hsm/ABCHsm.hpp"

TEST_F(AsyncHsm, multithreaded_entrypoint_cancelation)
{
    TEST_DESCRIPTION("entrypoint transitions should be atomic and can't be cancled");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<AsyncHsm>(AsyncHsmState::A, this, &AsyncHsm::onStateChanged, nullptr, &AsyncHsm::onExit);
    registerState<AsyncHsm>(AsyncHsmState::B, this, &AsyncHsm::onStateChanged);
    registerState<AsyncHsm>(AsyncHsmState::C, this, &AsyncHsm::onStateChanged);

    ASSERT_TRUE(registerSubstate(AsyncHsmState::P1, AsyncHsmState::B, true));

    registerTransition(AsyncHsmState::A, AsyncHsmState::P1, AsyncHsmEvent::NEXT_STATE);
    registerTransition(AsyncHsmState::P1, AsyncHsmState::C, AsyncHsmEvent::EXIT_SUBSTATE);
    registerTransition(AsyncHsmState::C, AsyncHsmState::A, AsyncHsmEvent::NEXT_STATE);

    ASSERT_EQ(getCurrentState(), AsyncHsmState::A);

    //-------------------------------------------
    // ACTIONS
    // this should trigger A -> [P1 -> B]
    transition(AsyncHsmEvent::NEXT_STATE);
    waitAsyncOperation(200);// wait for A::onExit

    // send new event with clearQueue=TRUE
    transitionWithQueueClear(AsyncHsmEvent::EXIT_SUBSTATE);

    unblockNextStep();// allow A::onExit to continue
    waitAsyncOperation(200);// wait for B::onStateChanged

    // NOTE: this is the main validation point. In case of an error state would be C since we would never go into B
    ASSERT_EQ(getCurrentState(), AsyncHsmState::B);
    unblockNextStep();// allow B::onStateChanged to continue

    waitAsyncOperation(200);// wait for C::onStateChanged
    ASSERT_EQ(getCurrentState(), AsyncHsmState::C);
    unblockNextStep();// allow C::onStateChanged to continue

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(getCurrentState(), AsyncHsmState::C);
}

TEST_F(ABCHsm, multithreaded_deleting_running_dispatcher)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    registerState<ABCHsm>(AbcState::A, this, &ABCHsm::onA);
    registerState<ABCHsm>(AbcState::B, this, &ABCHsm::onB);

    registerTransition(AbcState::A, AbcState::B, AbcEvent::E1);
    registerTransition(AbcState::B, AbcState::A, AbcEvent::E1);

    //-------------------------------------------
    // ACTIONS
    for (int i = 0; i < 100; ++i)
    {
        transition(AbcEvent::E1);
    }

    //-------------------------------------------
    // VALIDATION
}
