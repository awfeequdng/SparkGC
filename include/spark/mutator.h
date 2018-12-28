//
// Created by kiva on 2018-12-28.
//
#pragma once

#include <compileTimeConfig.h>

#if SPARK_ARCH_arm

#include <spark/arch/arm/atomic.h>

#elif SPARK_ARCH_aarch64

#include <spark/arch/aarch64/atomic.h>

#elif SPARK_ARCH_x86

#include <spark/arch/x86/atomic.h>

#elif SPARK_ARCH_x86_64

#include <spark/arch/x86_64/atomic.h>

#endif
