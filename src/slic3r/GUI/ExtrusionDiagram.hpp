#include <wx/wx.h>
#include <wx/collpane.h>


class ExtrusionDiagram : public wxPanel {
    public:
        ExtrusionDiagram(wxWindow* parent);

        void UpdateValues(double val, const std::string& config_option);

        void SetShowNozzle(bool state) { m_draw_nozzle = state; }
        void ResizeDiagram();
        void Inilize(){DrawExtrusion(); /*InitializeMinMaxPoints();*/ }
        void InitializeMinMaxPoints();
        wxSize GetDrawingBounds() const {

            if (m_min_point.x == INT_MAX || m_min_point.y == INT_MAX ||
                m_max_point.x == INT_MIN || m_max_point.y == INT_MIN) {
                return wxSize(1, 1);//drawings not yet created
            }

            int width = m_max_point.x + m_min_point.x;
            int height = m_max_point.y + m_min_point.y;
        
            int padding = 20;
            return wxSize(width + padding, height + padding);
        }

    private:
        void OnPaint(wxPaintEvent&);
        void OnResize(wxSizeEvent&);
        void DrawExtrusion();
        void DrawNozzle(wxDC& dc, double center_x, double extrusion_top_y, double extrusion_width, double base_scale);
        void DrawArrows(wxPaintDC& dc, wxPoint start, wxPoint end);
        wxPoint DrawText(wxPaintDC& dc, const wxPoint& position, wxString text, double scale);

        void UpdateMinMax(const wxPoint& bounds);
        //void InitializeMinMaxPoints();
        //void ResizeDiagram();


        double m_nozzle_diameter;
        double m_layer_height;
        double m_width;
        double m_overlap;
        double m_spacing;
        int    m_perimeters;
        wxCheckBox* m_show_nozzle_checkbox;
        bool m_draw_nozzle;

        wxPoint m_min_point;//stores min/max points since 'GetBestSize' doesn't work correctly with drawings.
        wxPoint m_max_point;
       
    };