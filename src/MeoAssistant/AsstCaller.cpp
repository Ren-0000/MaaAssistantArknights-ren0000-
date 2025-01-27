#include "AsstCaller.h"

#include <cstring>
#include <filesystem>
#include <iostream>

#include <meojson/json.hpp>

#include "Assistant.h"
#include "ResourceLoader.h"
#include "Utils/AsstTypes.h"
#include "Utils/Logger.hpp"
#include "Utils/UserDir.hpp"
#include "Utils/Version.h"

static constexpr AsstSize NullSize = static_cast<AsstSize>(-1);

#if 0
#if _MSC_VER
// Win32平台下Dll的入口
BOOL APIENTRY DllMain(HINSTANCE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#elif VA_GNUC

#endif
#endif

static bool inited = false;

bool AsstSetUserDir(const char* path)
{
    return asst::UserDir::get_instance().set(path);
}

bool AsstLoadResource(const char* path)
{
    if (auto& user_dir = asst::UserDir::get_instance(); user_dir.empty()) {
        user_dir.set(path);
    }
    using namespace asst::utils::path_literals;
    inited = asst::ResourceLoader::get_instance().load(asst::utils::path(path) / "resource"_p);
    return inited;
}

bool AsstSetProcessOption(AsstProcessOptionKey key, const char* value)
{
    // TODO: 把 config.option 里的全实现一遍
    std::ignore = key;
    std::ignore = value;
    return false;
}

AsstHandle AsstCreate()
{
    if (!inited) {
        return nullptr;
    }
    return new asst::Assistant();
}

AsstHandle AsstCreateEx(AsstApiCallback callback, void* custom_arg)
{
    if (!inited) {
        return nullptr;
    }
    return new asst::Assistant(callback, custom_arg);
}

void AsstDestroy(AsstHandle handle)
{
    if (handle == nullptr) {
        return;
    }

    delete handle;
    handle = nullptr;
}

bool AsstSetInstanceOption(AsstHandle handle, AsstInstanceOptionKey key, const char* value)
{
    if (handle == nullptr) {
        return false;
    }

    return handle->set_instance_option(static_cast<asst::InstanceOptionKey>(key), value);
}

bool AsstConnect(AsstHandle handle, const char* adb_path, const char* address, const char* config)
{
    if (!inited || handle == nullptr) {
        return false;
    }

    return handle->connect(adb_path, address, config ? config : std::string());
}

bool AsstStart(AsstHandle handle)
{
    if (!inited || handle == nullptr) {
        return false;
    }

    return handle->start();
}

bool AsstStop(AsstHandle handle)
{
    if (!inited || handle == nullptr) {
        return false;
    }

    return handle->stop();
}

bool AsstRunning(AsstHandle handle)
{
    if (!inited || handle == nullptr) {
        return false;
    }

    return handle->running();
}

AsstTaskId AsstAppendTask(AsstHandle handle, const char* type, const char* params)
{
    if (!inited || handle == nullptr) {
        return 0;
    }

    return handle->append_task(type, params ? params : "");
}

bool AsstSetTaskParams(AsstHandle handle, AsstTaskId id, const char* params)
{
    if (!inited || handle == nullptr) {
        return false;
    }

    return handle->set_task_params(id, params ? params : "");
}

bool AsstClick(AsstHandle handle, int x, int y)
{
    if (!inited || handle == nullptr) {
        return false;
    }
    return handle->ctrler_click(x, y);
}

AsstSize AsstGetImage(AsstHandle handle, void* buff, AsstSize buff_size)
{
    if (!inited || handle == nullptr || buff == nullptr) {
        return NullSize;
    }
    auto img_data = handle->get_image();
    size_t data_size = img_data.size();
    if (buff_size < data_size) {
        return NullSize;
    }
    memcpy(buff, img_data.data(), data_size * sizeof(decltype(img_data)::value_type));
    return data_size;
}

AsstSize AsstGetUUID(AsstHandle handle, char* buff, AsstSize buff_size)
{
    if (!inited || handle == nullptr || buff == nullptr) {
        return NullSize;
    }
    auto uuid = handle->get_uuid();
    size_t data_size = uuid.size();
    if (buff_size < data_size) {
        return NullSize;
    }
    memcpy(buff, uuid.data(), data_size * sizeof(decltype(uuid)::value_type));
    return data_size;
}

AsstSize AsstGetTasksList(AsstHandle handle, AsstTaskId* buff, AsstSize buff_size)
{
    if (!inited || handle == nullptr || buff == nullptr) {
        return NullSize;
    }
    auto tasks = handle->get_tasks_list();
    size_t data_size = tasks.size();
    if (buff_size < data_size) {
        return NullSize;
    }
    memcpy(buff, tasks.data(), data_size * sizeof(decltype(tasks)::value_type));
    return data_size;
}

AsstSize AsstGetNullSize()
{
    return NullSize;
}

const char* AsstGetVersion()
{
    return asst::Version;
}

void AsstLog(const char* level, const char* message)
{
    if (!inited) {
        std::cerr << "Not inited" << std::endl;
        return;
    }
    asst::Log.log(asst::Logger::level(level), message);
}
