// Webview Panel created by Steve.
#include "PrinterWebView.hpp"

#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "libslic3r_version.h"

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/toolbar.h>
#include <wx/textdlg.h>

#include "CR3DWebView.hpp"
#include <wx/webview.h>
#include <wx/msw/webview_edge.h>
#include <wx/msw/webview_ie.h>

// Todo add if mac

#if _MSW_
#include <WebView2.h>
#endif

namespace Slic3r {
namespace GUI {

WebViewPanel::WebViewPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    m_webView = CreateWebView(parent);
    sizer->Add(m_webView, 1, wxEXPAND);
}

WebViewPanel::~WebViewPanel(){}

wxWebView* WebViewPanel::CreateWebView(wxWindow* parent) {

    wxWebView *webview = WebView::CreateWebView(parent, "");

    webview->EnableContextMenu(true);
    webview->EnableAccessToDevTools(true);

    return webview;
}

void WebViewPanel::load_url(wxString &url)
{
    if (m_webView == nullptr)
        return;

    m_webView->LoadURL(url);

    if (url.IsEmpty()) {
        std::cout << "URL is empty";
    }
}


} // namespace GUI

} // namespace Slic3r
