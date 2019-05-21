/*
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/helpers/hw_helper_base.inl"

namespace NEO {

template <typename GfxFamily>
void HwHelperHw<GfxFamily>::adjustDefaultEngineType(HardwareInfo *pHwInfo) {
}

template <typename GfxFamily>
uint32_t HwHelperHw<GfxFamily>::getComputeUnitsUsedForScratch(const HardwareInfo *pHwInfo) const {
    return pHwInfo->gtSystemInfo.MaxSubSlicesSupported * pHwInfo->gtSystemInfo.MaxEuPerSubSlice *
           pHwInfo->gtSystemInfo.ThreadCount / pHwInfo->gtSystemInfo.EUCount;
}

template <typename GfxFamily>
void HwHelperHw<GfxFamily>::setCapabilityCoherencyFlag(const HardwareInfo *pHwInfo, bool &coherencyFlag) {
    coherencyFlag = true;
}

template <typename GfxFamily>
uint32_t HwHelperHw<GfxFamily>::getConfigureAddressSpaceMode() {
    return 0u;
}

template <typename GfxFamily>
bool HwHelperHw<GfxFamily>::isLocalMemoryEnabled(const HardwareInfo &hwInfo) const {
    return false;
}

template <typename GfxFamily>
bool HwHelperHw<GfxFamily>::supportsYTiling() const {
    return true;
}

template <typename GfxFamily>
bool HwHelperHw<GfxFamily>::timestampPacketWriteSupported() const {
    return false;
}

template <typename GfxFamily>
const std::vector<aub_stream::EngineType> HwHelperHw<GfxFamily>::getGpgpuEngineInstances() const {
    constexpr std::array<aub_stream::EngineType, 2> gpgpuEngineInstances = {{aub_stream::ENGINE_RCS,
                                                                             aub_stream::ENGINE_RCS}}; // low priority
    return std::vector<aub_stream::EngineType>(gpgpuEngineInstances.begin(), gpgpuEngineInstances.end());
}

template <typename GfxFamily>
std::string HwHelperHw<GfxFamily>::getExtensions() const {
    return "";
}

} // namespace NEO