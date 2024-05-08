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

#ifndef DGL_WEB_VIEW_HPP_INCLUDED
#define DGL_WEB_VIEW_HPP_INCLUDED

#include "TopLevelWidget.hpp"

// --------------------------------------------------------------------------------------------------------------------

// TODO private data
START_NAMESPACE_DISTRHO

struct WebViewData;
typedef WebViewData* WebViewHandle;

END_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

START_NAMESPACE_DGL

// --------------------------------------------------------------------------------------------------------------------
// WebViewWidget

class WebViewWidget : public TopLevelWidget
{
public:
   /**
      Constructor for a WebViewWidget.
    */
    explicit WebViewWidget(Window& windowToMapTo);

   /**
      Destructor.
    */
    ~WebViewWidget() override;

    // webview methods
    void evaluateJS(const char* js);
    void reload();

protected:
    virtual void onMessage(char* message);
    void onResize(const ResizeEvent& ev) override;

private:
    const DISTRHO_NAMESPACE::WebViewHandle webview;
    void onDisplay() override {}

    // TODO inside private data
    static void _on_msg(void*, char*);

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewWidget)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DGL

#endif // DGL_WEB_VIEW_HPP_INCLUDED
