/*
 * Copyright (C) 2017-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/command_queue/command_queue_hw.h"
#include "runtime/event/event.h"
#include "runtime/memory_manager/memory_manager.h"
#include "test.h"
#include "unit_tests/command_queue/command_queue_fixture.h"
#include "unit_tests/command_queue/enqueue_fixture.h"
#include "unit_tests/fixtures/buffer_fixture.h"
#include "unit_tests/fixtures/device_fixture.h"
#include "unit_tests/fixtures/hello_world_kernel_fixture.h"
#include "unit_tests/fixtures/memory_management_fixture.h"
#include "unit_tests/fixtures/simple_arg_kernel_fixture.h"

using namespace NEO;

struct OOMSetting {
    bool oomCS;
    bool oomISH;
};

static OOMSetting oomSettings[] = {
    {true, false},
    {false, true},
    {true, true}};

struct OOMCommandQueueBufferTest : public MemoryManagementFixture,
                                   public DeviceFixture,
                                   public CommandQueueFixture,
                                   public SimpleArgKernelFixture,
                                   public HelloWorldKernelFixture,
                                   public ::testing::TestWithParam<OOMSetting> {

    using CommandQueueFixture::SetUp;
    using HelloWorldKernelFixture::SetUp;
    using SimpleArgKernelFixture::SetUp;

    OOMCommandQueueBufferTest() {
    }

    void SetUp() override {
        MemoryManagement::breakOnAllocationEvent = 77;
        MemoryManagementFixture::SetUp();
        DeviceFixture::SetUp();
        context = new MockContext(pClDevice);
        BufferDefaults::context = context;
        CommandQueueFixture::SetUp(context, pClDevice, 0);
        SimpleArgKernelFixture::SetUp(pClDevice);
        HelloWorldKernelFixture::SetUp(pClDevice, "CopyBuffer_simd", "CopyBuffer");

        srcBuffer = BufferHelper<>::create();
        dstBuffer = BufferHelper<>::create();

        const auto &oomSetting = GetParam();
        auto oomSize = 10u;
        if (oomSetting.oomCS) {
            auto &cs = pCmdQ->getCS(oomSize);

            // CommandStream may be larger than requested so grab what wasnt requested
            cs.getSpace(cs.getAvailableSpace() - oomSize);
            ASSERT_EQ(oomSize, cs.getAvailableSpace());
        }

        if (oomSetting.oomISH) {
            auto &ish = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, oomSize);

            // IndirectHeap may be larger than requested so grab what wasnt requested
            ish.getSpace(ish.getAvailableSpace() - oomSize);
            ASSERT_EQ(oomSize, ish.getAvailableSpace());
        }
    }

    void TearDown() override {
        delete dstBuffer;
        delete srcBuffer;
        context->release();
        HelloWorldKernelFixture::TearDown();
        SimpleArgKernelFixture::TearDown();
        CommandQueueFixture::TearDown();
        DeviceFixture::TearDown();
        MemoryManagementFixture::TearDown();
    }

    MockContext *context;
    Buffer *srcBuffer = nullptr;
    Buffer *dstBuffer = nullptr;
};

HWTEST_P(OOMCommandQueueBufferTest, enqueueCopyBuffer) {
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueCopyBufferHelper<>::enqueue(pCmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal1);

    auto retVal2 = EnqueueCopyBufferHelper<>::enqueue(&cmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal2);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }
}

HWTEST_P(OOMCommandQueueBufferTest, enqueueFillBuffer) {
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueFillBufferHelper<>::enqueue(pCmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal1);

    auto retVal2 = EnqueueFillBufferHelper<>::enqueue(&cmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal2);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }
}

HWTEST_P(OOMCommandQueueBufferTest, enqueueReadBuffer) {
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueReadBufferHelper<>::enqueue(pCmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal1);

    auto retVal2 = EnqueueReadBufferHelper<>::enqueue(&cmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal2);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }
}

HWTEST_P(OOMCommandQueueBufferTest, enqueueWriteBuffer) {
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueWriteBufferHelper<>::enqueue(pCmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal1);

    auto retVal2 = EnqueueWriteBufferHelper<>::enqueue(&cmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal2);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }
}

HWTEST_P(OOMCommandQueueBufferTest, enqueueWriteBufferRect) {
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueWriteBufferRectHelper<>::enqueue(pCmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal1);

    auto retVal2 = EnqueueWriteBufferRectHelper<>::enqueue(&cmdQ);
    EXPECT_EQ(CL_SUCCESS, retVal2);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }
}

HWTEST_P(OOMCommandQueueBufferTest, enqueueKernelHelloWorld) {
    typedef HelloWorldKernelFixture KernelFixture;
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueKernelHelper<>::enqueueKernel(
        pCmdQ,
        KernelFixture::pKernel);

    auto retVal2 = EnqueueKernelHelper<>::enqueueKernel(
        &cmdQ,
        KernelFixture::pKernel);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }

    EXPECT_EQ(CL_SUCCESS, retVal1);
    EXPECT_EQ(CL_SUCCESS, retVal2);
}

HWTEST_P(OOMCommandQueueBufferTest, enqueueKernelSimpleArg) {
    typedef SimpleArgKernelFixture KernelFixture;
    CommandQueueHw<FamilyType> cmdQ(context, pClDevice, 0, false);

    auto &commandStream = pCmdQ->getCS(1024);
    auto &indirectHeap = pCmdQ->getIndirectHeap(IndirectHeap::DYNAMIC_STATE, 10);
    auto usedBeforeCS = commandStream.getUsed();
    auto usedBeforeISH = indirectHeap.getUsed();

    auto retVal1 = EnqueueKernelHelper<>::enqueueKernel(
        pCmdQ,
        KernelFixture::pKernel);

    auto retVal2 = EnqueueKernelHelper<>::enqueueKernel(
        &cmdQ,
        KernelFixture::pKernel);

    auto usedAfterCS = commandStream.getUsed();
    auto usedAfterISH = indirectHeap.getUsed();
    EXPECT_LE(usedAfterCS - usedBeforeCS, commandStream.getMaxAvailableSpace());
    if (usedAfterISH > usedBeforeISH) {
        EXPECT_LE(usedAfterISH - usedBeforeISH, indirectHeap.getMaxAvailableSpace());
    } else {
        EXPECT_LE(usedAfterISH, indirectHeap.getMaxAvailableSpace());
    }

    EXPECT_EQ(CL_SUCCESS, retVal1);
    EXPECT_EQ(CL_SUCCESS, retVal2);
}

INSTANTIATE_TEST_CASE_P(
    OOM,
    OOMCommandQueueBufferTest,
    testing::ValuesIn(oomSettings));
