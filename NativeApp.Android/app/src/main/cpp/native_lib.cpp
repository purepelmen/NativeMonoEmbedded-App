#include <filesystem>

#include <jni.h>
#include <android/asset_manager_jni.h>

#include <shared_main.h>
#include "android_mono.h"

using std::filesystem::path;

static path GetPathFromJString(JNIEnv *env, jstring jstr);

extern "C"
JNIEXPORT jint JNICALL
Java_com_monoembedtest_nativeapp_MonoRuntimeBootstrap_start(JNIEnv *env, jclass clazz,
                                                            jstring exePathJStr, jobject assetManager)
{
    path exePath = GetPathFromJString(env, exePathJStr);
    AndroidMono_Init(exePath, AAssetManager_fromJava(env, assetManager));

    return sharedNative_main(exePath);
}

static path GetPathFromJString(JNIEnv *env, jstring jstr)
{
    jboolean isCopy = false;

    auto chars = env->GetStringUTFChars(jstr, &isCopy);
    path path{ chars };

    if (isCopy)
        env->ReleaseStringUTFChars(jstr, chars);

    return path;
}