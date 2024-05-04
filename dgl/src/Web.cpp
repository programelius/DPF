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

#include "../Web.hpp"

#include "../distrho/extra/WebView.hpp"

#include "TopLevelWidgetPrivateData.hpp"
#include "WindowPrivateData.hpp"

START_NAMESPACE_DGL

// -----------------------------------------------------------------------

WebViewWidget::WebViewWidget(Window& windowToMapTo)
    : TopLevelWidget(windowToMapTo),
      webview(addWebView(windowToMapTo.getNativeWindowHandle(),
                         0, 0,
                         windowToMapTo.getWidth(),
                         windowToMapTo.getHeight(),
                         windowToMapTo.getScaleFactor())) {}

WebViewWidget::~WebViewWidget()
{
    if (webview != nullptr)
        destroyWebView(webview);
}

void WebViewWidget::onResize(const ResizeEvent& ev)
{
    TopLevelWidget::onResize(ev);

    if (webview != nullptr)
        resizeWebView(webview, 0, 0, ev.size.getWidth(), ev.size.getHeight());
}

// -----------------------------------------------------------------------

static void notImplemented(const char* const name)
{
    d_stderr2("web function not implemented: %s", name);
}

// -----------------------------------------------------------------------

void TopLevelWidget::PrivateData::display()
{
}

// -----------------------------------------------------------------------

void Window::PrivateData::renderToPicture(const char*, const GraphicsContext&, uint, uint)
{
    notImplemented("Window::PrivateData::renderToPicture");
}

// -----------------------------------------------------------------------

const GraphicsContext& Window::PrivateData::getGraphicsContext() const noexcept
{
    return (const GraphicsContext&)graphicsContext;
}

// -----------------------------------------------------------------------

END_NAMESPACE_DGL

// -----------------------------------------------------------------------
