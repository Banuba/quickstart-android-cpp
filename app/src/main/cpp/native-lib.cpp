
#include <bnb/utils/interfaces/all.hpp>
#include <bnb/effect_player/interfaces/all.hpp>

#include <jni.h>

#include <string>
#include <vector>

using namespace bnb;
using namespace bnb::interfaces;

namespace
{
    std::string jstring2string(JNIEnv* env, jstring jstr)
    {
        const char* chars = env->GetStringUTFChars(jstr, NULL);
        std::string ret(chars);
        env->ReleaseStringUTFChars(jstr, chars);
        return ret;
    }

    struct BanubaSdkManager
    {
        std::shared_ptr<effect_player> effectPlayer;

        BanubaSdkManager()
            : effectPlayer{effect_player::create({
                720 /*fx_width*/,
                1280 /*fx_height*/,
                nn_mode::automatically /*nn_enable*/,
                face_search_mode::good /*face_search*/,
                false /*js_debugger_enable*/,
                false /*manual_audio*/
            })}
        {
        }
    };
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_initialize(JNIEnv* env, jobject thiz, jstring path_to_resources, jstring client_token)
{
    std::vector<std::string> paths{jstring2string(env, path_to_resources)};
    auto token = jstring2string(env, client_token);


    utility_manager::initialize(paths, token);
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_deinitialize(JNIEnv* env, jobject thiz)
{
    utility_manager::release();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_createEffectPlayer(JNIEnv* env, jobject thiz)
{
    auto sdk = new BanubaSdkManager();
    return (jlong) sdk;
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_destroyEffectPlayer(JNIEnv* env, jobject thiz, jlong effect_player)
{
    auto sdk = (BanubaSdkManager*) effect_player;
    delete sdk;
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_loadEffect(JNIEnv* env, jobject thiz, jlong effect_player, jstring name)
{
    auto sdk = (BanubaSdkManager*) effect_player;
    sdk->effectPlayer->load_effect(jstring2string(env, name));
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_processPhoto(JNIEnv* env, jobject thiz, jlong effect_player, jobject rgba, jint width, jint height)
{
    auto sdk = (BanubaSdkManager*) effect_player;

    auto size = static_cast<size_t>(env->GetDirectBufferCapacity(rgba));
    auto* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(rgba));

    bnb::full_image_t image{
        bpc8_image_t{
            color_plane_weak(data),
            bpc8_image_t::pixel_format_t::rgba,
            image_format{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                bnb::camera_orientation::deg_0,
                /* is_mirrored */ false,
                /* face_orientation */ 0}}};

    const auto processed = sdk->effectPlayer->process_image(
        std::move(image),
        /* output_format */ pixel_format::rgba,
        /* process_image_params */ {false, {}});

    auto byte_array = env->NewByteArray(processed.size);
    env->SetByteArrayRegion(byte_array, 0, processed.size, reinterpret_cast<const jbyte*>(processed.data.get()));
    return byte_array;
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_surfaceCreated(JNIEnv* env, jobject thiz, jlong effect_player, jint width, jint height)
{
    auto sdk = (BanubaSdkManager*) effect_player;
    sdk->effectPlayer->surface_created(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_surfaceChanged(JNIEnv* env, jobject thiz, jlong effect_player, jint width, jint height)
{
    auto sdk = (BanubaSdkManager*) effect_player;
    sdk->effectPlayer->surface_changed(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_surfaceDestroyed(JNIEnv* env, jobject thiz, jlong effect_player)
{
    auto sdk = (BanubaSdkManager*) effect_player;
    sdk->effectPlayer->surface_destroyed();
}
