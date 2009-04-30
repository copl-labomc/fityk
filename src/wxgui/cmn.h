// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License version 2 or (at your option) 3
// $Id$
#ifndef FITYK_WX_CMN_H_
#define FITYK_WX_CMN_H_

#include <string>
#include <vector>
#include <wx/splitter.h>
#include <wx/arrstr.h>
#include <wx/spinctrl.h>
#include <wx/mstream.h>

// input mode, changes functions of left and right clicks
enum MouseModeEnum { mmd_zoom, mmd_bg, mmd_add, mmd_range, mmd_peak };

// plot types, e.g. to tell where is mouse
enum PlotTypeEnum { pte_main, pte_aux };


inline wxString pchar2wx(char const* pc)
{
    return wxString(pc, wxConvUTF8);
}

inline wxString s2wx(std::string const& s) { return pchar2wx(s.c_str()); }


inline std::string wx2s(wxString const& w) 
{ 
    return std::string((const char*) w.mb_str(wxConvUTF8)); 
}

inline wxArrayString stl2wxArrayString(std::vector<std::string> const& vs)
{
    wxArrayString wxas; 
    for (std::vector<std::string>::const_iterator i = vs.begin(); 
                                                        i != vs.end(); ++i)
        wxas.Add(s2wx(*i));
    return wxas;
}

// convenient wxArrayString constructors
inline wxArrayString ArrayString(wxString const& s1) 
{ 
    wxArrayString a(1, &s1);
    return a;
} 
inline wxArrayString ArrayString(wxString const& s1, wxString const& s2) 
{ 
    wxArrayString a(1, &s1);
    a.Add(s2);
    return a;
} 
inline wxArrayString ArrayString(wxString const& s1, wxString const& s2, 
                                 wxString const& s3) 
{ 
    wxArrayString a(1, &s1);
    a.Add(s2);
    a.Add(s3);
    return a;
} 

/// wxTextCtrl for real number input, will be enhanced 
class RealNumberCtrl : public wxTextCtrl
{
public:
    RealNumberCtrl(wxWindow* parent, wxWindowID id, wxString const& value)
        : wxTextCtrl(parent, id, value) {}
    RealNumberCtrl(wxWindow* parent, wxWindowID id, std::string const& value)
        : wxTextCtrl(parent, id, s2wx(value)) {}
    RealNumberCtrl(wxWindow* parent, wxWindowID id, double value)
        : wxTextCtrl(parent, id, wxString::Format(wxT("%g"), value)) {}
};


// only wxString and long types can be read conveniently from wxConfig
// these functions are defined in plot.cpp

class wxConfigBase;
class wxString;

bool cfg_read_bool(wxConfigBase* cf, wxString const& key, bool def_val);
double cfg_read_double(wxConfigBase* cf, wxString const& key, double def_val);

wxColour cfg_read_color(wxConfigBase const* config, wxString const& key,
                        wxColour const& default_value);
void cfg_write_color(wxConfigBase* config, const wxString& key,
                           const wxColour& value);

wxFont cfg_read_font(wxConfigBase const* config, wxString const& key,
                             wxFont const& default_value);
void cfg_write_font(wxConfigBase* config, wxString const& key,
                          wxFont const& value);

bool should_focus_input(wxKeyEvent& event);

bool change_color_dlg(wxColour& col);
void add_apply_close_buttons(wxWindow *parent, wxSizer *top_sizer);

//dummy events -- useful when calling event handler functions
extern wxMouseEvent dummy_mouse_event;
extern wxCommandEvent dummy_cmd_event;

/// based on http://wiki.wxpython.org/index.cgi/ProportionalSplitterWindow
/// it is like wxSplitterWindow, but when resized, both windows are resized
/// proporionally
class ProportionalSplitter: public wxSplitterWindow
{
public:
    ProportionalSplitter(wxWindow* parent, 
                         wxWindowID id=-1, 
                         float proportion=0.66, // 0. - 1.
                         const wxSize& size = wxDefaultSize,
                         long style=wxSP_NOBORDER|wxSP_3DSASH);
    bool SplitHorizontally(wxWindow* win1, wxWindow* win2, float proportion=-1);
    bool SplitVertically(wxWindow* win1, wxWindow* win2, float proportion=-1);
    float GetProportion() const { return m_proportion; }
    void SetProportion(float proportion) {m_proportion=proportion; ResetSash();}
    void SetSashPosition(int position);

protected:
    float m_proportion; //0-1
    bool m_firstpaint;

    void ResetSash();
    int GetExpectedSashPosition();
    void OnReSize(wxSizeEvent& event);
    void OnSashChanged(wxSplitterEvent &event);
    void OnPaint(wxPaintEvent &event);
};

class SpinCtrl: public wxSpinCtrl
{
public:
    SpinCtrl(wxWindow* parent, wxWindowID id, int val, 
             int min, int max, int width=50)
        : wxSpinCtrl (parent, id, wxString::Format(wxT("%i"), val),
                      wxDefaultPosition, wxSize(width, -1), 
                      wxSP_ARROW_KEYS, min, max, val) 
    {}
};

/// wxTextCtrl which sends wxEVT_COMMAND_TEXT_ENTER when loses the focus
class KFTextCtrl : public wxTextCtrl
{
public:
    KFTextCtrl(wxWindow* parent, wxWindowID id, wxString const& value,
               int size=-1, long style=0) 
        : wxTextCtrl(parent, id, value, wxDefaultPosition, wxSize(size, -1),
                     style | wxTE_PROCESS_ENTER) {}
    void OnKillFocus(wxFocusEvent&);
    DECLARE_EVENT_TABLE()
};

/// get path ~/.fityk/filename and create ~/.fityk/ dir if not exists
wxString get_user_conffile(std::string const& filename);


/// introduced because OnOK and OnCancel were removed;
/// when wx-2.6 compatibility is dropped, it will be replaced with 
/// wxSetEscapeId for Cancel and perhaps with wxSetAffirmativeId for OK.
inline void close_it(wxDialog* dlg, int id=wxID_CANCEL)
{
    if (dlg->IsModal())
        dlg->EndModal(id);
    else {
        dlg->SetReturnCode(id);
        dlg->Show(false);
    }
}

// same as cwi->Clear(), cwi->Append(...), but optimized for some special cases
void updateControlWithItems(wxControlWithItems *cwi, 
                            std::vector<std::string> const& v);

# if !wxCHECK_VERSION(2, 9, 0)
#define wxPENSTYLE_SHORT_DASH wxSHORT_DASH
#define wxPENSTYLE_DOT wxDOT
#define wxPENSTYLE_DOT_DASH wxDOT_DASH
typedef int wxPenStyle;
#endif


// from http://www.wxwidgets.org/wiki/index.php/Embedding_PNG_Images
inline wxBitmap GetBitmapFromMemory_(const unsigned char *data, int length) 
{
    wxMemoryInputStream is(data, length);
    return wxBitmap(wxImage(is, wxBITMAP_TYPE_PNG));
}

#define GET_BMP(name) \
            GetBitmapFromMemory_(name##_png, sizeof(name##_png))

// add note in static box
wxSizer* note_sizer(wxWindow *parent, wxString const& title, 
                    wxString const& msg);
// add persistance note (use GUI > Save...)
wxSizer* persistance_note_sizer(wxWindow *parent);

#endif // FITYK_WX_CMN_H_
