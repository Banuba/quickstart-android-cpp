
#include <bnb/effect_player.h>
#include <bnb/error.h>
#include <bnb/utility_manager.h>

#include "offscreen_render_target.hpp"
#include "thread_pool.h"

#include <android/log.h>
#include <jni.h>

#include <string>
#include <thread>
#include <vector>


#define CHECK_ERROR(error)                                  \
    do {                                                    \
        if (error) {                                        \
            std::string msg = bnb_error_get_message(error); \
            bnb_error_destroy(error);                       \
            throw std::runtime_error(msg);                  \
        }                                                   \
    } while (false);

utility_manager_holder_t* utility;
bnb::offscreen_render_target* ort;

struct data_t
{
    using type = uint8_t[];
    using pointer = uint8_t*;
    using uptr = std::unique_ptr<type, std::function<void(pointer)>>;
    uptr data;
    size_t size;
};

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
        effect_player_holder_t* effectPlayer;
        bnb::thread_pool scheduler = bnb::thread_pool(1);
        std::thread::id render_thread_id;

        BanubaSdkManager()
        {
            // Size of photo.jpg
            int32_t width = 1000;
            int32_t height = 1500;
            bnb_effect_player_configuration_t ep_cfg{width, height, bnb_nn_mode_automatically, bnb_good, false, false};
            effectPlayer = bnb_effect_player_create(&ep_cfg, nullptr);
            bnb_effect_player_enable_audio(effectPlayer, false, nullptr);
            ort = new bnb::offscreen_render_target(width, height);

            auto task = [this, width, height]() {
                render_thread_id = std::this_thread::get_id();
                ort->init();
                ort->activate_context();
            };

            auto future = scheduler.enqueue(task);
            try {
                // Wait result of task since initialization of glad can cause exceptions if proceed without
                future.get();
            }
            catch (std::runtime_error& e) {
                std::cout << "[ERROR] Failed to initialize effect player: " << e.what() << std::endl;
                throw std::runtime_error("Failed to initialize effect player.");
            }
        }

        ~BanubaSdkManager()
        {
            auto task = []() {
                ort->deinit();
            };
            scheduler.enqueue(task).get();

            delete ort;
        }
    };
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_initialize(JNIEnv* env, jobject thiz, jstring path_to_resources, jstring client_token)
{
    std::vector<std::string> paths{jstring2string(env, path_to_resources)};
    auto token = jstring2string(env, client_token);

    std::unique_ptr<const char* []> res_paths = std::make_unique<const char* []>(paths.size() + 1);
    std::transform(paths.begin(), paths.end(), res_paths.get(), [](const auto& s) { return s.c_str(); });
    res_paths.get()[paths.size()] = nullptr;

    utility = bnb_utility_manager_init(res_paths.get(), token.c_str(), nullptr);
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_deinitialize(JNIEnv* env, jobject thiz)
{
    bnb_utility_manager_release(utility, nullptr);
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
    bnb_effect_player_surface_destroyed(sdk->effectPlayer, nullptr);
    bnb_effect_player_destroy(sdk->effectPlayer, nullptr);
    delete sdk;
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_loadEffect(JNIEnv* env, jobject thiz, jlong effect_player, jstring name)
{
    auto sdk = (BanubaSdkManager*) effect_player;
    auto effect_name = jstring2string(env, name);

    auto task = [sdk, effect_name]() {
        ort->activate_context();

        if (auto e_manager = bnb_effect_player_get_effect_manager(sdk->effectPlayer, nullptr)) {
            bnb_effect_manager_load_effect(e_manager, effect_name.c_str(), nullptr);
        } else {
            __android_log_print(ANDROID_LOG_ERROR, "", "effect manager not initialized");
        }
    };
    sdk->scheduler.enqueue(task).get();
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_processPhoto(JNIEnv* env, jobject thiz, jlong effect_player, jobject rgba, jint width, jint height)
{
    auto sdk = (BanubaSdkManager*) effect_player;

    auto size = static_cast<size_t>(env->GetDirectBufferCapacity(rgba));
    auto* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(rgba));

    uint32_t chanels = 4;

    bnb_image_format_t image_format{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                bnb_image_orientation_t::BNB_DEG_0,
                /* is_mirrored */ false,
                /* face_orientation */ 0};

    bnb_error* error = nullptr;
    full_image_holder_t* img = bnb_full_image_from_bpc8_img(image_format, bnb_pixel_format_t::BNB_RGBA, data, static_cast<uint32_t>(width) * chanels, &error);
    CHECK_ERROR(error);
    if (!img) {
        throw std::runtime_error("no image was created");
    }

    auto task = [sdk, img]() {
        ort->activate_context();
        ort->prepare_rendering();

        bnb_error* error = nullptr;
        bnb_effect_player_push_frame(sdk->effectPlayer, img, &error);
        CHECK_ERROR(error);
        bnb_full_image_release(img, nullptr);
        CHECK_ERROR(error);

        while (bnb_effect_player_draw(sdk->effectPlayer, &error) < 0) {
            std::this_thread::yield();
        }
        CHECK_ERROR(error);

        ort->orient_image(bnb::orient_format{bnb_image_orientation_t::BNB_DEG_0, true});

        return ort->read_current_buffer();
    };

    auto data_r = sdk->scheduler.enqueue(task).get();

    auto byte_array = env->NewByteArray(data_r.size);
    env->SetByteArrayRegion(byte_array, 0, data_r.size, reinterpret_cast<const jbyte*>(data_r.data.get()));
    return byte_array;
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_surfaceCreated(JNIEnv* env, jobject thiz, jlong effect_player, jint width, jint height)
{
    auto sdk = (BanubaSdkManager*) effect_player;

    auto task = [sdk, width, height]() {
        ort->activate_context();
        bnb_effect_player_surface_created(sdk->effectPlayer, width, height, nullptr);
    };
    sdk->scheduler.enqueue(task).get();
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_surfaceChanged(JNIEnv* env, jobject thiz, jlong effect_player, jint width, jint height)
{
    auto sdk = (BanubaSdkManager*) effect_player;

    auto task = [sdk, width, height]() {
        ort->activate_context();

        bnb_effect_player_surface_changed(sdk->effectPlayer, width, height, nullptr);
        effect_manager_holder_t* em = bnb_effect_player_get_effect_manager(sdk->effectPlayer, nullptr);
        bnb_effect_manager_set_effect_size(em, width, height, nullptr);

        ort->surface_changed(width, height);
    };
    sdk->scheduler.enqueue(task).get();
}

extern "C" JNIEXPORT void JNICALL
Java_com_banuba_sdk_example_quickstart_1cpp_BanubaSdk_surfaceDestroyed(JNIEnv* env, jobject thiz, jlong effect_player)
{
    auto sdk = (BanubaSdkManager*) effect_player;

    auto task = [sdk]() {
        bnb_effect_player_surface_destroyed(sdk->effectPlayer, nullptr);
    };
    sdk->scheduler.enqueue(task).get();
}
