#pragma once
#if 1
#define DETECTION_DLL_EXPORT
#else
#ifdef DETECTION_BUILD_DLL
#define DETECTION_DLL_EXPORT __declspec(dllexport)
#else
#define DETECTION_DLL_EXPORT __declspec(dllimport)
#endif
#endif

