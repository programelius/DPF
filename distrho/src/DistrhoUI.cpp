/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2024 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoDetails.hpp"
#include "src/DistrhoPluginChecks.h"
#include "src/DistrhoDefines.h"

#include <cstddef>

#ifdef DISTRHO_PROPER_CPP11_SUPPORT
# include <cstdint>
#else
# include <stdint.h>
#endif

#if DISTRHO_UI_FILE_BROWSER && !defined(DISTRHO_OS_MAC)
# define DISTRHO_PUGL_NAMESPACE_MACRO_HELPER(NS, SEP, FUNCTION) NS ## SEP ## FUNCTION
# define DISTRHO_PUGL_NAMESPACE_MACRO(NS, FUNCTION) DISTRHO_PUGL_NAMESPACE_MACRO_HELPER(NS, _, FUNCTION)
# define x_fib_add_recent          DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_add_recent)
# define x_fib_cfg_buttons         DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_cfg_buttons)
# define x_fib_cfg_filter_callback DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_cfg_filter_callback)
# define x_fib_close               DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_close)
# define x_fib_configure           DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_configure)
# define x_fib_filename            DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_filename)
# define x_fib_free_recent         DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_free_recent)
# define x_fib_handle_events       DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_handle_events)
# define x_fib_load_recent         DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_load_recent)
# define x_fib_recent_at           DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_recent_at)
# define x_fib_recent_count        DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_recent_count)
# define x_fib_recent_file         DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_recent_file)
# define x_fib_save_recent         DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_save_recent)
# define x_fib_show                DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_show)
# define x_fib_status              DISTRHO_PUGL_NAMESPACE_MACRO(plugin, x_fib_status)
# define DISTRHO_FILE_BROWSER_DIALOG_HPP_INCLUDED
# define FILE_BROWSER_DIALOG_NAMESPACE DISTRHO_NAMESPACE
# define FILE_BROWSER_DIALOG_DISTRHO_NAMESPACE
START_NAMESPACE_DISTRHO
# include "../extra/FileBrowserDialogImpl.hpp"
END_NAMESPACE_DISTRHO
# include "../extra/FileBrowserDialogImpl.cpp"
#endif

#if DISTRHO_UI_USE_WEBVIEW && !defined(DISTRHO_OS_MAC)
# define DISTRHO_WEB_VIEW_HPP_INCLUDED
# define WEB_VIEW_NAMESPACE DISTRHO_NAMESPACE
# define WEB_VIEW_DISTRHO_NAMESPACE
START_NAMESPACE_DISTRHO
# include "../extra/WebViewImpl.hpp"
END_NAMESPACE_DISTRHO
# include "../extra/WebViewImpl.cpp"
#endif

#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
# if defined(DISTRHO_OS_WINDOWS)
#  include <winsock2.h>
#  include <windows.h>
# elif defined(HAVE_X11)
#  include <X11/Xresource.h>
# endif
#else
# include "src/TopLevelWidgetPrivateData.hpp"
# include "src/WindowPrivateData.hpp"
#endif

#include "DistrhoUIPrivateData.hpp"

START_NAMESPACE_DISTRHO

/* ------------------------------------------------------------------------------------------------------------
 * Static data, see DistrhoUIInternal.hpp */

const char* g_nextBundlePath  = nullptr;
#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
uintptr_t   g_nextWindowId    = 0;
double      g_nextScaleFactor = 1.0;
#endif

#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
/* ------------------------------------------------------------------------------------------------------------
 * get global scale factor */

#ifdef DISTRHO_OS_MAC
double getDesktopScaleFactor(uintptr_t parentWindowHandle);
#else
static double getDesktopScaleFactor(const uintptr_t parentWindowHandle)
{
    // allow custom scale for testing
    if (const char* const scale = getenv("DPF_SCALE_FACTOR"))
        return std::max(1.0, std::atof(scale));

#if defined(DISTRHO_OS_WINDOWS)
    if (const HMODULE Shcore = LoadLibraryA("Shcore.dll"))
    {
        typedef HRESULT(WINAPI* PFN_GetProcessDpiAwareness)(HANDLE, DWORD*);
        typedef HRESULT(WINAPI* PFN_GetScaleFactorForMonitor)(HMONITOR, DWORD*);

# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
        const PFN_GetProcessDpiAwareness GetProcessDpiAwareness
            = (PFN_GetProcessDpiAwareness)GetProcAddress(Shcore, "GetProcessDpiAwareness");
        const PFN_GetScaleFactorForMonitor GetScaleFactorForMonitor
            = (PFN_GetScaleFactorForMonitor)GetProcAddress(Shcore, "GetScaleFactorForMonitor");
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif

        DWORD dpiAware = 0;
        DWORD scaleFactor = 100;
        if (GetProcessDpiAwareness && GetScaleFactorForMonitor
            && GetProcessDpiAwareness(nullptr, &dpiAware) == 0 && dpiAware != 0)
        {
            const HMONITOR hMon = parentWindowHandle != 0
                                ? MonitorFromWindow((HWND)parentWindowHandle, MONITOR_DEFAULTTOPRIMARY)
                                : MonitorFromPoint(POINT{0,0}, MONITOR_DEFAULTTOPRIMARY);
            GetScaleFactorForMonitor(hMon, &scaleFactor);
        }

        FreeLibrary(Shcore);
        return static_cast<double>(scaleFactor) / 100.0;
    }
#elif defined(HAVE_X11)
    ::Display* const display = XOpenDisplay(nullptr);
    DISTRHO_SAFE_ASSERT_RETURN(display != nullptr, 1.0);

    XrmInitialize();

    double dpi = 96.0;
    if (char* const rms = XResourceManagerString(display))
    {
        if (const XrmDatabase db = XrmGetStringDatabase(rms))
        {
            char* type = nullptr;
            XrmValue value = {};

            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)
                && type != nullptr
                && std::strcmp(type, "String") == 0
                && value.addr != nullptr)
            {
                char*        end    = nullptr;
                const double xftDpi = std::strtod(value.addr, &end);
                if (xftDpi > 0.0 && xftDpi < HUGE_VAL)
                    dpi = xftDpi;
            }

            XrmDestroyDatabase(db);
        }
    }

    XCloseDisplay(display);
    return dpi / 96;
#endif

    return 1.0;

    // might be unused
    (void)parentWindowHandle;
}
#endif // !DISTRHO_OS_MAC

#endif

/* ------------------------------------------------------------------------------------------------------------
 * UI::PrivateData special handling */

UI::PrivateData* UI::PrivateData::s_nextPrivateData = nullptr;

#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
ExternalWindow::PrivateData
#else
PluginWindow&
#endif
UI::PrivateData::createNextWindow(UI* const ui, uint width, uint height, const bool adjustForScaleFactor)
{
    UI::PrivateData* const pData = s_nextPrivateData;
   #if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    const double scaleFactor = d_isNotZero(pData->scaleFactor) ? pData->scaleFactor : getDesktopScaleFactor(pData->winId);

    if (adjustForScaleFactor && d_isNotZero(scaleFactor) && d_isNotEqual(scaleFactor, 1.0))
    {
        width *= scaleFactor;
        height *= scaleFactor;
    }

    pData->window = new PluginWindow(ui, pData->app);
    ExternalWindow::PrivateData ewData;
    ewData.parentWindowHandle = pData->winId;
    ewData.width = width;
    ewData.height = height;
    ewData.scaleFactor = scaleFactor;
    ewData.title = DISTRHO_PLUGIN_NAME;
    ewData.isStandalone = DISTRHO_UI_IS_STANDALONE;
    return ewData;
   #else
    const double scaleFactor = pData->scaleFactor;

    if (adjustForScaleFactor && d_isNotZero(scaleFactor) && d_isNotEqual(scaleFactor, 1.0))
    {
        width *= scaleFactor;
        height *= scaleFactor;
    }

    pData->window = new PluginWindow(ui, pData->app, pData->winId, width, height, scaleFactor);

    // If there are no callbacks, this is most likely a temporary window, so ignore idle callbacks
    if (pData->callbacksPtr == nullptr)
        pData->window->setIgnoreIdleCallbacks();

    return pData->window.getObject();
   #endif
}

/* ------------------------------------------------------------------------------------------------------------
 * UI */

UI::UI(const uint width, const uint height, const bool automaticallyScaleAndSetAsMinimumSize)
    : UIWidget(UI::PrivateData::createNextWindow(this,
               // width
              #ifdef DISTRHO_UI_DEFAULT_WIDTH
               width == 0 ? DISTRHO_UI_DEFAULT_WIDTH :
              #endif
               width,
               // height
              #ifdef DISTRHO_UI_DEFAULT_HEIGHT
               height == 0 ? DISTRHO_UI_DEFAULT_HEIGHT :
              #endif
               height,
               // adjustForScaleFactor
              #ifdef DISTRHO_UI_DEFAULT_WIDTH
               width == 0
              #else
               false
              #endif
               )
              #if DISTRHO_UI_WEB_VIEW
               , true
              #endif
               ),
      uiData(UI::PrivateData::s_nextPrivateData)
{
#if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    if (width != 0 && height != 0)
    {
        Widget::setSize(width, height);

        if (automaticallyScaleAndSetAsMinimumSize)
            setGeometryConstraints(width, height, true, true, true);
    }
   #ifdef DISTRHO_UI_DEFAULT_WIDTH
    else
    {
        Widget::setSize(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT);
    }
   #endif
#else
    // unused
    (void)automaticallyScaleAndSetAsMinimumSize;
#endif

  #if DISTRHO_UI_WEB_VIEW
    init(
"editParameter=function(index,started){window.webkit.messageHandlers.external.postMessage('editparam '+index+' '+(started ? 1 : 0))};"
"setParameterValue=function(index,value){window.webkit.messageHandlers.external.postMessage('setparam '+index+' '+value)};"
   #if DISTRHO_PLUGIN_WANT_STATE
"setState=function(key,value){window.webkit.messageHandlers.external.postMessage('setstate '+key+' '+value)};"
"requestStateFile=function(key){window.webkit.messageHandlers.external.postMessage('reqstatefile '+key)};"
   #endif
   #if DISTRHO_PLUGIN_WANT_MIDI_INPUT
"sendNote=function(channel,note,velocity){window.webkit.messageHandlers.external.postMessage('sendnote '+channel+' '+note+' '+velocity)};"
   #endif
    );
  #endif
}

UI::~UI()
{
}

/* ------------------------------------------------------------------------------------------------------------
 * Host state */

bool UI::isResizable() const noexcept
{
#if DISTRHO_UI_USER_RESIZABLE
# if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    return true;
# else
    return uiData->window->isResizable();
# endif
#else
    return false;
#endif
}

uint UI::getBackgroundColor() const noexcept
{
    return uiData->bgColor;
}

uint UI::getForegroundColor() const noexcept
{
    return uiData->fgColor;
}

double UI::getSampleRate() const noexcept
{
    return uiData->sampleRate;
}

const char* UI::getBundlePath() const noexcept
{
    return uiData->bundlePath;
}

void UI::editParameter(uint32_t index, bool started)
{
    uiData->editParamCallback(index + uiData->parameterOffset, started);
}

void UI::setParameterValue(uint32_t index, float value)
{
    uiData->setParamCallback(index + uiData->parameterOffset, value);
}

#if DISTRHO_PLUGIN_WANT_STATE
void UI::setState(const char* key, const char* value)
{
    uiData->setStateCallback(key, value);
}
#endif

#if DISTRHO_PLUGIN_WANT_STATE
bool UI::requestStateFile(const char* key)
{
    return uiData->fileRequestCallback(key);
}
#endif

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
void UI::sendNote(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uiData->sendNoteCallback(channel, note, velocity);
}
#endif

#if DISTRHO_UI_FILE_BROWSER
bool UI::openFileBrowser(const FileBrowserOptions& options)
{
    return getWindow().openFileBrowser((DGL_NAMESPACE::FileBrowserOptions&)options);
}
#endif

#if DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
/* ------------------------------------------------------------------------------------------------------------
 * Direct DSP access */

void* UI::getPluginInstancePointer() const noexcept
{
    return uiData->dspPtr;
}
#endif

#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
/* ------------------------------------------------------------------------------------------------------------
 * External UI helpers (static calls) */

const char* UI::getNextBundlePath() noexcept
{
    return g_nextBundlePath;
}

double UI::getNextScaleFactor() noexcept
{
    return g_nextScaleFactor;
}

# if DISTRHO_PLUGIN_HAS_EMBED_UI
uintptr_t UI::getNextWindowId() noexcept
{
    return g_nextWindowId;
}
# endif
#endif // DISTRHO_PLUGIN_HAS_EXTERNAL_UI

/* ------------------------------------------------------------------------------------------------------------
 * DSP/Plugin Callbacks */

void UI::parameterChanged(const uint32_t index, const float value)
{
   #if DISTRHO_UI_WEB_VIEW
    char msg[128];
    {
        const ScopedSafeLocale ssl;
        std::snprintf(msg, sizeof(msg) - 1,
                      "typeof(parameterChanged) === 'function' && parameterChanged(%u,%f)", index, value);
    }
    evaluateJS(msg);
   #else
    // unused
    (void)index;
    (void)value;
   #endif
}

#if DISTRHO_PLUGIN_WANT_PROGRAMS
void UI::programLoaded(const uint32_t index)
{
   #if DISTRHO_UI_WEB_VIEW
    char msg[128];
    std::snprintf(msg, sizeof(msg) - 1,
                  "typeof(programLoaded) === 'function' && programLoaded(%u)", index);
    evaluateJS(msg);
   #else
    // unused
    (void)index;
   #endif
}
#endif

#if DISTRHO_PLUGIN_WANT_STATE
void UI::stateChanged(const char* const key, const char* const value)
{
   #if DISTRHO_UI_WEB_VIEW
    const size_t keylen = std::strlen(key);
    const size_t valuelen = std::strlen(value);
    const size_t msglen = keylen + valuelen + 60;
    if (char* const msg = static_cast<char*>(std::malloc(msglen)))
    {
        // TODO escape \\'
        std::snprintf(msg, sizeof(msglen) - 1,
                      "typeof(stateChanged) === 'function' && stateChanged('%s','%s')", key, value);
        msg[msglen - 1] = '\0';
        evaluateJS(msg);
        std::free(msg);
    }
   #else
    // unused
    (void)key;
    (void)value;
   #endif
}
#endif

/* ------------------------------------------------------------------------------------------------------------
 * DSP/Plugin Callbacks (optional) */

void UI::sampleRateChanged(const double sampleRate)
{
   #if DISTRHO_UI_WEB_VIEW
    char msg[128];
    {
        const ScopedSafeLocale ssl;
        std::snprintf(msg, sizeof(msg) - 1,
                      "typeof(sampleRateChanged) === 'function' && sampleRateChanged(%f)", sampleRate);
    }
    evaluateJS(msg);
   #else
    // unused
    (void)sampleRate;
   #endif
}

/* ------------------------------------------------------------------------------------------------------------
 * UI Callbacks (optional) */

void UI::uiScaleFactorChanged(double)
{
}

#if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
std::vector<DGL_NAMESPACE::ClipboardDataOffer> UI::getClipboardDataOfferTypes()
{
    return uiData->window->getClipboardDataOfferTypes();
}

uint32_t UI::uiClipboardDataOffer()
{
    std::vector<DGL_NAMESPACE::ClipboardDataOffer> offers(uiData->window->getClipboardDataOfferTypes());

    for (std::vector<DGL_NAMESPACE::ClipboardDataOffer>::iterator it=offers.begin(), end=offers.end(); it != end;++it)
    {
        const DGL_NAMESPACE::ClipboardDataOffer offer = *it;
        if (std::strcmp(offer.type, "text/plain") == 0)
            return offer.id;
    }

    return 0;
}

void UI::uiFocus(bool, DGL_NAMESPACE::CrossingMode)
{
}

void UI::uiReshape(const uint width, const uint height)
{
    // NOTE this must be the same as Window::onReshape
    pData->fallbackOnResize(width, height);
}
#endif // !DISTRHO_PLUGIN_HAS_EXTERNAL_UI

#if DISTRHO_UI_FILE_BROWSER
void UI::uiFileBrowserSelected(const char*)
{
}
#endif

/* ------------------------------------------------------------------------------------------------------------
 * UI Resize Handling, internal */

#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
void UI::sizeChanged(const uint width, const uint height)
{
    UIWidget::sizeChanged(width, height);

    uiData->setSizeCallback(width, height);
}
#else
void UI::onResize(const ResizeEvent& ev)
{
    UIWidget::onResize(ev);

   #if ! DISTRHO_UI_USES_SIZE_REQUEST
    if (uiData->initializing)
        return;

    const uint width = ev.size.getWidth();
    const uint height = ev.size.getHeight();
    uiData->setSizeCallback(width, height);
   #endif
}

// NOTE: only used for CLAP and VST3
void UI::requestSizeChange(const uint width, const uint height)
{
   #if DISTRHO_UI_USES_SIZE_REQUEST
    if (uiData->initializing)
        uiData->window->setSizeFromHost(width, height);
    else
        uiData->setSizeCallback(width, height);
   #else
    // unused
    (void)width;
    (void)height;
   #endif
}
#endif

// -----------------------------------------------------------------------------------------------------------

#if DISTRHO_UI_WEB_VIEW
void UI::onMessage(char* const message)
{
    if (std::strncmp(message, "setparam ", 9) == 0)
    {
        const char* const strindex = message + 9;
        char* strvalue = nullptr;
        const ulong index = std::strtoul(strindex, &strvalue, 10);
        DISTRHO_SAFE_ASSERT_RETURN(strvalue != nullptr && strindex != strvalue,);

        float value;
        {
            const ScopedSafeLocale ssl;
            value = std::atof(strvalue);
        }
        setParameterValue(index, value);
        return;
    }

    if (std::strncmp(message, "editparam ", 10) == 0)
    {
        const char* const strindex = message + 10;
        char* strvalue = nullptr;
        const ulong index = std::strtoul(strindex, &strvalue, 10);
        DISTRHO_SAFE_ASSERT_RETURN(strvalue != nullptr && strindex != strvalue,);

        const bool started = strvalue[0] != '0';
        editParameter(index, started);
        return;
    }

   #if DISTRHO_PLUGIN_WANT_STATE
    if (std::strncmp(message, "setstate ", 9) == 0)
    {
        char* const key = message + 9;
        char* const sep = std::strchr(key, ' ');
        DISTRHO_SAFE_ASSERT_RETURN(sep != nullptr,);
        *sep = 0;
        char* const value = sep + 1;

        setState(key, value);
        return;
    }

    if (std::strncmp(message, "reqstatefile ", 13) == 0)
    {
        const char* const key = message + 13;
        requestStateFile(key);
        return;
    }
   #endif

   #if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    if (std::strncmp(message, "sendnote ", 9) == 0)
    {
        const char* const strchannel = message + 9;
        char* strnote = nullptr;
        char* strvelocity = nullptr;
        char* end = nullptr;

        const ulong channel = std::strtoul(strchannel, &strnote, 10);
        DISTRHO_SAFE_ASSERT_RETURN(strnote != nullptr && strchannel != strnote,);

        const ulong note = std::strtoul(strnote, &strvelocity, 10);
        DISTRHO_SAFE_ASSERT_RETURN(strvelocity != nullptr && strchannel != strvelocity,);

        const ulong velocity = std::strtoul(strvelocity, &end, 10);
        DISTRHO_SAFE_ASSERT_RETURN(end != nullptr && strvelocity != end,);

        sendNote(channel, note, started);
        return;
    }
   #endif

    d_stderr("UI received unknown message %s", message);
}
#endif

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
