// Sample03_RectangleClip.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DemoApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE /* hPrevInstance */,
    _In_ LPWSTR    /* lpCmdLine */,
    _In_ int       /* nCmdShow */)
{
    // Ignore the return value because we want to run the program even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    if (SUCCEEDED(::CoInitialize(NULL)))
    {
        DemoApp::GlobalInit(hInstance);
        DemoApp app;

        if (SUCCEEDED(app.Initialize()))
        {
            app.RunMessageLoop();
        }

        ::CoUninitialize();
    }

    return 0;
}
