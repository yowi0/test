#include <Windows.h>
#include <iostream>
#include <fstream>
#include "IL2CPP.hpp"

namespace IL2CPP
{
    HMODULE GameAssembly_handle = nullptr;

    Domain* CurrentDomain()
    {
        return reinterpret_cast<Domain*>(IMPORT::il2cpp_domain_get());
    }

    void AttachCurrentThread()
    {
        IMPORT::il2cpp_thread_attach(IMPORT::il2cpp_domain_get());
    }

    HMODULE GetGameAssemblyModule()
    {
        HMODULE hModule = GetModuleHandleA("GameAssembly.dll");
        if (!hModule)
        {
            std::cerr << "[ERROR] GameAssembly.dll not found!\n";
            return nullptr;
        }
        std::cout << "[+] GameAssembly.dll loaded at: 0x" << hModule << std::endl;
        return hModule;
    }

    void INIT()
    {
        static bool il2cppInitialized = false;

        if (il2cppInitialized)
        {
            std::cout << "[!] IL2CPP was already initialized.\n";
            return;
        }

        std::cout << "[+] Initializing IL2CPP...\n";

        GameAssembly_handle = GetGameAssemblyModule();
        if (!GameAssembly_handle)
        {
            return;
        }

        INIT_API_FUNCTIONS(GameAssembly_handle);
        VM_FUNCTIONS::INIT();
        AttachCurrentThread();
        CommonCShrap::INIT();

        il2cppInitialized = true;
        std::cout << "[+] IL2CPP successfully initialized.\n";

        if (!IMPORT::il2cpp_domain_get || !IMPORT::il2cpp_thread_attach)
        {
            std::cerr << "[ERROR] Required IL2CPP functions not found!\n";
            return;
        }
    }
}

void FindCurrencyOffset()
{
    IL2CPP::Domain* domain = IL2CPP::CurrentDomain();
    if (!domain)
    {
        std::cerr << "[ERROR] Failed to get IL2CPP domain.\n";
        return;
    }
    std::cout << "[+] IL2CPP domain found at: 0x" << domain << std::endl;

    IL2CPP::Image* assemblyCSharp_image = domain->OpenAssembly("Assembly-CSharp.dll");
    if (!assemblyCSharp_image)
    {
        std::cerr << "[ERROR] Failed to load Assembly-CSharp.dll\n";
        return;
    }
    std::cout << "[+] Assembly-CSharp.dll loaded at: 0x" << assemblyCSharp_image << std::endl;

    const std::string moneyManager_namespace = "Framework";
    const std::string moneyManager_keyname = "MoneyManager";

    IL2CPP::FieldPattern ClassofCurrency = {
        "String", "String", "String", "String", "String", "String", "String", "String", "String", "String",
        "List1", "List1", "List1", "List1", "List1", "List1",
        "Boolean", "Boolean", "Boolean", "Boolean", "Boolean",
        "Dictionary2", "Dictionary2", "Dictionary2", "Dictionary2",
        "Action1", "Action1", "Action1", "Action1",
        "Coroutine",
        "Int32", "Int32", "Int32", "Int32", "Int32", "Int32",
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
    };

    IL2CPP::SignaturePattern AddCurrency_Pattern{
        "internal void",
        nullptr,
        nullptr,
        { "String", "Int32", "Enum", "Boolean", "Boolean", "Int32" }
    };

    bool mappingSuccess = false;

    try
    {
        IL2CPP::ClassMapping::AddImageToScan(assemblyCSharp_image);
        IL2CPP::ClassMapping::AddQueue(moneyManager_keyname, moneyManager_namespace, &ClassofCurrency);
        IL2CPP::ClassMapping::StartMapping();

        mappingSuccess = true;
    }
    catch (IL2CPP::Exception::UnresolvedMappingQueue& err)
    {
        std::cout << err.what() << std::endl;
    }

    if (mappingSuccess)
    {
        IL2CPP::Class* moneyManager_class = IL2CPP::ClassMapping::GetClass(moneyManager_keyname);
        if (!moneyManager_class)
        {
            std::cerr << "[ERROR] Failed to find MoneyManager class.\n";
            return;
        }
        std::cout << "[+] MoneyManager class found at: 0x" << moneyManager_class << std::endl;

        uintptr_t methodAddr = reinterpret_cast<uintptr_t>(moneyManager_class->GetMethodByPattern(AddCurrency_Pattern));
        if (!methodAddr)
        {
            std::cerr << "[ERROR] Failed to locate AddCurrency method.\n";
            return;
        }
        std::cout << "[+] AddCurrency method located at: 0x" << std::hex << methodAddr << std::dec << std::endl;
    }
}

void CreateConsole()
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    SetConsoleTitleA("Debug Console - Test DLL");
    SetForegroundWindow(GetConsoleWindow());

    std::cout << "[+] Debug console successfully created.\n";
}

void DestroyConsole()
{
    FreeConsole();
}

DWORD WINAPI InitThread(LPVOID param)
{
    CreateConsole();
    IL2CPP::INIT();
    FindCurrencyOffset();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        DestroyConsole();
        break;
    }
    return TRUE;
}
