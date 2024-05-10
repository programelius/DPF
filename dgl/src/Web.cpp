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
#include "../Color.hpp"

#include "../distrho/extra/WebView.hpp"
#include "../distrho/extra/Sleep.hpp"

#include "SubWidgetPrivateData.hpp"
#include "TopLevelWidgetPrivateData.hpp"
#include "WindowPrivateData.hpp"

START_NAMESPACE_DGL

// --------------------------------------------------------------------------------------------------------------------

WebViewWidget::WebViewWidget(Window& windowToMapTo)
    : TopLevelWidget(windowToMapTo),
      webview(nullptr)
{
}

WebViewWidget::~WebViewWidget()
{
    if (webview != nullptr)
    {
       #if !(defined(DISTRHO_OS_MAC) || defined(DISTRHO_OS_WINDOWS))
        removeIdleCallback(this);
       #endif
        webViewDestroy(webview);
    }
}

void WebViewWidget::init(const char* const url, const char* const initialJS)
{
    DISTRHO_SAFE_ASSERT_RETURN(webview == nullptr,);

    WebViewOptions options(_on_msg, this);
    options.initialJS = initialJS;
    webview = webViewCreate(url, getWindow().getNativeWindowHandle(), getWidth(), getHeight(), getScaleFactor(), options);

   #if !(defined(DISTRHO_OS_MAC) || defined(DISTRHO_OS_WINDOWS))
    if (webview != nullptr)
        addIdleCallback(this, 1000 / 60);
   #endif
}

void WebViewWidget::evaluateJS(const char* const js)
{
    if (webview != nullptr)
        webViewEvaluateJS(webview, js);
}

void WebViewWidget::reload()
{
    if (webview != nullptr)
        webViewReload(webview);
}

void WebViewWidget::_on_msg(void* const arg, char* const message)
{
    static_cast<WebViewWidget*>(arg)->onMessage(message);
}

void WebViewWidget::onMessage(char*)
{
}

void WebViewWidget::onResize(const ResizeEvent& ev)
{
    TopLevelWidget::onResize(ev);

    if (webview != nullptr)
        webViewResize(webview, ev.size.getWidth(), ev.size.getHeight(), getScaleFactor());
}

void WebViewWidget::idleCallback()
{
    webViewIdle(webview);
}

// --------------------------------------------------------------------------------------------------------------------

static void notImplemented(const char* const name)
{
    d_stderr2("web function not implemented: %s", name);
}

// -----------------------------------------------------------------------
// Color

void Color::setFor(const GraphicsContext&, bool)
{
    notImplemented("Color::setFor");
}

// -----------------------------------------------------------------------
// Line

template<typename T>
void Line<T>::draw(const GraphicsContext&, T)
{
    notImplemented("Line::draw");
}

template<typename T>
void Line<T>::draw()
{
    notImplemented("Line::draw");
}

template class Line<double>;
template class Line<float>;
template class Line<int>;
template class Line<uint>;
template class Line<short>;
template class Line<ushort>;

// -----------------------------------------------------------------------
// Circle

template<typename T>
void Circle<T>::draw(const GraphicsContext&)
{
    notImplemented("Circle::draw");
}

template<typename T>
void Circle<T>::drawOutline(const GraphicsContext&, T)
{
    notImplemented("Circle::drawOutline");
}

template<typename T>
void Circle<T>::draw()
{
    notImplemented("Circle::draw");
}

template<typename T>
void Circle<T>::drawOutline()
{
    notImplemented("Circle::drawOutline");
}

template class Circle<double>;
template class Circle<float>;
template class Circle<int>;
template class Circle<uint>;
template class Circle<short>;
template class Circle<ushort>;

// -----------------------------------------------------------------------
// Triangle

template<typename T>
void Triangle<T>::draw(const GraphicsContext&)
{
    notImplemented("Triangle::draw");
}

template<typename T>
void Triangle<T>::drawOutline(const GraphicsContext&, T)
{
    notImplemented("Triangle::drawOutline");
}

template<typename T>
void Triangle<T>::draw()
{
    notImplemented("Triangle::draw");
}

template<typename T>
void Triangle<T>::drawOutline()
{
    notImplemented("Triangle::drawOutline");
}

template class Triangle<double>;
template class Triangle<float>;
template class Triangle<int>;
template class Triangle<uint>;
template class Triangle<short>;
template class Triangle<ushort>;


// -----------------------------------------------------------------------
// Rectangle

template<typename T>
void Rectangle<T>::draw(const GraphicsContext&)
{
    notImplemented("Rectangle::draw");
}

template<typename T>
void Rectangle<T>::drawOutline(const GraphicsContext&, T)
{
    notImplemented("Rectangle::drawOutline");
}

template<typename T>
void Rectangle<T>::draw()
{
    notImplemented("Rectangle::draw");
}

template<typename T>
void Rectangle<T>::drawOutline()
{
    notImplemented("Rectangle::drawOutline");
}

template class Rectangle<double>;
template class Rectangle<float>;
template class Rectangle<int>;
template class Rectangle<uint>;
template class Rectangle<short>;
template class Rectangle<ushort>;

// -----------------------------------------------------------------------

void SubWidget::PrivateData::display(uint, uint, double)
{
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
