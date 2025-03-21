#include "ExtrusionDiagram.hpp"
#include "slic3r/GUI/I18N.hpp"

ExtrusionDiagram::ExtrusionDiagram(wxWindow* parent)
    : wxPanel(parent)
{
    
    Bind(wxEVT_PAINT, &ExtrusionDiagram::OnPaint, this);
    //Bind(wxEVT_SIZE, &ExtrusionDiagram::OnResize, this);
    //OnPaint();
    //DrawExtrusion();
    //OnResize();
    InitializeMinMaxPoints();
    //ResizeDiagram();
    //CallAfter([=]() {
    //    ResizeDiagram();
    //});

}

void ExtrusionDiagram::OnPaint(wxPaintEvent&) {

    wxPaintDC dc(this);
    dc.Clear();

    DrawExtrusion();

    double x_offset = 10;
    double y_offset = 20;
    wxPoint textPosition(x_offset, y_offset);
    double fontSize = 10;

    textPosition = DrawText(dc, wxPoint( x_offset, textPosition.y), _L("** NOT TO SCALE **"), fontSize);
    textPosition.y = textPosition.y + y_offset;
    textPosition = DrawText(dc, wxPoint( x_offset, textPosition.y), _L("adjust values: nozzle_diameter, layer_height, external_perimeter_extrusion_width, external_perimeter_overlap, external_perimeter_extrusion_spacing, perimeters"), fontSize);
    textPosition.y = textPosition.y + y_offset;
    textPosition = DrawText(dc, wxPoint( x_offset, textPosition.y), _L("then click on the 'update drwaing button' to get a visual clue on how that setting changes things."), fontSize);
    textPosition.y = textPosition.y + y_offset;
    textPosition = DrawText(dc, wxPoint( x_offset, textPosition.y), _L("If the perimeter overlap is set at 100%, the yellow areas should be filled by the overlap.\nIf overlap is at 0%, width = spacing."), fontSize);
    
    //TODO: add more text and arrows point at stuff


    //textPosition.y = textPosition.y + y_offset;
    //textPosition = DrawText(dc, wxPoint( x_offset, textPosition.y), _L("** NOT TO SCALE **"), fontSize);

}

void ExtrusionDiagram::DrawExtrusion(){

    wxPaintDC dc(this);
    dc.Clear();

    double base_scale = m_draw_nozzle ? 300.0 : 800.0;
    const double extrusion_width = (m_width * base_scale) / 2;
    const double nozzle_scale = m_nozzle_diameter * base_scale;// FIXME: nozzle scale is adjusted wrong.. this should be used as a orifice scale adjustment, not the full drawing.
    const double extrusion_height = m_layer_height * base_scale;
    const double radius = extrusion_height / 2.5;
    const int perimeters = m_perimeters;

    // Calculate overlap and step distance (X-axis scaling only)
    double overlap_shift = radius * m_overlap;
    double step_distance = extrusion_width - overlap_shift;

    double start_x = m_draw_nozzle ? 80.0 : 50.0;
    double start_y = m_draw_nozzle ? 500.0 : 500.0;

    std::vector<double> extrusion_positions;

    for (int i = 0; i < perimeters; ++i) {
        double extrusion_x = start_x + step_distance * i;
        extrusion_positions.push_back(extrusion_x);
    }
    if (extrusion_positions.empty()) {
        return;
    }

    // Draw yellow overlaps first (so they appear behind)
    for (int i = 0; i < perimeters - 1; ++i) {

        double extrusion1_x = extrusion_positions[i];
        double extrusion2_x = extrusion_positions[i + 1];

        // Position halfway between the two extrusions
        double overlap_x = extrusion1_x + extrusion_width - (extrusion_width / 2);
        
        // Calculate the visible gap extrusion_width
        double yellow_width = extrusion2_x - overlap_x;

        if (yellow_width > 0) {
            dc.SetBrush(wxBrush(wxColour(255, 255, 0))); // Yellow color
            dc.SetPen(*wxTRANSPARENT_PEN);
            start_y = m_draw_nozzle ? 500.0 : 10.0;
            dc.DrawRectangle(overlap_x, start_y, yellow_width * 2, extrusion_height);
        }
    }

    // Draw extrusions on top
    for (int i = 0; i < perimeters; ++i) {

        double extrusion_x = extrusion_positions[i];
        dc.SetBrush(wxBrush(wxColour(139, 69, 19))); // Brown color
        dc.SetPen(wxPen(*wxBLACK, 2)); // Black outline
        start_y = m_draw_nozzle ? 500.0 : 10.0;
        dc.DrawRoundedRectangle(extrusion_x, start_y, extrusion_width, extrusion_height, radius);
        //dc.DrawRoundedRectangle(extrusion_positions[perimeters - 1 - i], start_y, extrusion_width , extrusion_height, radius); doesn't work, drawings overwrite each other going backwards/forwards
        
        if(i == perimeters -1){

            for (int k = perimeters - 1; k >= 0; --k) {

                dc.SetBrush(*wxTRANSPARENT_BRUSH); // No fill color
                dc.DrawRoundedRectangle(extrusion_positions[k], start_y, extrusion_width, extrusion_height, radius);
            }
        }

        start_y = m_draw_nozzle ? 500.0 : 10.0;
        UpdateMinMax(wxPoint(extrusion_x, start_y));
        UpdateMinMax(wxPoint(extrusion_x + extrusion_width, start_y + extrusion_height));

    }

    double nozzle_width = 6, nozzle_height = 8;
    double center_of_first_extrusion =  (extrusion_positions[0] + extrusion_width) / 2;

    if (m_draw_nozzle) {
        double center_x = extrusion_positions[0] + (extrusion_width / 2);
        double extrusion_top_y = start_y;// top Y position of the extrusion
        DrawNozzle(dc, center_x, extrusion_top_y, nozzle_scale, base_scale);
    }


}
    
void ExtrusionDiagram::DrawNozzle(wxDC& dc, double center_x, double extrusion_top_y, double nozzle_scale, double base_scale) {
    // Scale factors
    double nozzle_tip_width = nozzle_scale;
    //double nozzle_tip_width = base_scale * nozzle_scale * 0.5;
    double nozzle_tip_height = base_scale * nozzle_scale * 0.0010;
    double nozzle_mid_height = base_scale * nozzle_scale * 0.0035;
    double nozzle_top_height = base_scale * nozzle_scale * 0.008;

    // Width multipliers for taper
    double nozzle_mid_width = nozzle_tip_width * 1.5;
    double nozzle_top_width = nozzle_mid_width * 0.8;

    // Y positions
    double nozzle_tip_y = nozzle_tip_width * 4;
    double nozzle_mid_y = nozzle_tip_y - nozzle_tip_height;
    double nozzle_top_y = nozzle_mid_y - nozzle_mid_height;

    //top left of panel is 0,0
    //distances are in pixels points
    //    | |    TOP
    //  __| |__  TOP
    //  |_   _|  MID
    //    \_/    TIP
    // total points = 12


    // --- Nozzle Tip (Trapezoid) ---
    wxPoint nozzle_tip_points[4] = {
        wxPoint(center_x - (nozzle_tip_width / 2), extrusion_top_y),  // Top left
        wxPoint(center_x + (nozzle_tip_width / 2), extrusion_top_y),  // Top right
        wxPoint(center_x + (nozzle_mid_width / 2), nozzle_mid_y),     // Bottom right
        wxPoint(center_x - (nozzle_mid_width / 2), nozzle_mid_y)      // Bottom left
    };
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawPolygon(4, nozzle_tip_points);

    // --- Middle Section (Rectangle) ---
    double mid_taper_offset = nozzle_mid_width * 0.3; // Increase for wider bottom taper
    wxPoint nozzle_mid_points[4] = {
        wxPoint(nozzle_tip_points[0].x - mid_taper_offset, nozzle_tip_points[3].y), // Top left
        wxPoint(nozzle_tip_points[1].x + mid_taper_offset, nozzle_tip_points[2].y), // Top right
        wxPoint(nozzle_tip_points[1].x + mid_taper_offset, nozzle_top_y),           // Bottom right
        wxPoint(nozzle_tip_points[0].x - mid_taper_offset, nozzle_top_y)            // Bottom left
    };
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawPolygon(4, nozzle_mid_points);

    // --- Top Section (Rectangle) ---
    double top_taper_offset = nozzle_top_width * 0.2; // Adjust this value to control taper
    wxPoint nozzle_top_points[4] = {
        wxPoint(nozzle_tip_points[3].x + top_taper_offset, nozzle_tip_points[3].y - nozzle_top_height), // Top left
        wxPoint(nozzle_tip_points[2].x - top_taper_offset, nozzle_tip_points[2].y - nozzle_top_height), // Top right
        wxPoint(nozzle_tip_points[2].x - top_taper_offset, nozzle_tip_points[2].y),                     // Bottom right
        wxPoint(nozzle_tip_points[3].x + top_taper_offset, nozzle_tip_points[3].y)                      // Bottom left
    };
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawPolygon(4, nozzle_top_points);


    for (const auto& point : nozzle_tip_points) {
        UpdateMinMax(point);
    }

    for (const auto& point : nozzle_mid_points) {
        UpdateMinMax(point);
    }

    for (const auto& point : nozzle_top_points) {
        UpdateMinMax(point);
    }

    //ResizeDiagram();
}

void ExtrusionDiagram::DrawArrows(wxPaintDC& dc, wxPoint start, wxPoint end) {
    dc.DrawLine(start, end);

    // Simple arrowhead
    int arrow_size = 5;
    dc.DrawLine(end.x, end.y, end.x - arrow_size, end.y - arrow_size);
    dc.DrawLine(end.x, end.y, end.x - arrow_size, end.y + arrow_size);

    UpdateMinMax(start);
    UpdateMinMax(end);
}

wxPoint ExtrusionDiagram::DrawText(wxPaintDC& dc, const wxPoint& position, wxString text, double fontSize) {

    if (!m_draw_nozzle) return wxPoint(0,0);

    // Create a scalable font based on the scale factor
    wxFont font(fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(font);//does this adjust for localizations?

    dc.SetTextForeground(*wxBLACK);
    dc.DrawText(text, position);
    
    int textWidth, textHeight;
    dc.GetTextExtent(text, &textWidth, &textHeight);// Calculate the text bounds for min/max update

    // Define the rectangle bounds of the text
    wxPoint topLeft(position.x, position.y);
    wxPoint bottomRight(position.x + textWidth, position.y + textHeight);

    UpdateMinMax(topLeft);
    UpdateMinMax(bottomRight);

    return topLeft;
}

void ExtrusionDiagram::OnResize(wxSizeEvent&) {
    //Refresh();
   //ResizeDiagram();
}

void ExtrusionDiagram::ResizeDiagram() {

    wxSize new_size = GetDrawingBounds();

    // Avoid redundant resizing
    //if (!newSize.IsFullySpecified() || GetSize() == newSize) {
    //    return;
    //}

    // Set minimum size for the sizer to respect layout
    SetMinSize(new_size);
    SetMaxSize(new_size);//hack fix stops tab-layout/updates from extending it to right
    SetSize(new_size);

}

void ExtrusionDiagram::UpdateValues(double val, const std::string& config_option) {
    bool updated = false;


    if (config_option == "nozzle_diameter") {
        m_nozzle_diameter = val;
        updated = true;
    }
    else if (config_option == "layer_height") {
        m_layer_height = val;
        updated = true;
    }
    else if (config_option == "external_perimeter_extrusion_width") {
        m_width = (m_width != 0) ? val : m_nozzle_diameter;
        //m_width = val;
        updated = true;
    }
    else if (config_option == "external_perimeter_overlap") {
        m_overlap = val;
        updated = true;
    } 
    else if (config_option == "external_perimeter_extrusion_spacing") {
        m_spacing = val;
        updated = true;
    }
    else if (config_option == "perimeters") {
        m_perimeters = val;
        updated = true;
    }
    else {
        wxLogWarning("Unknown config option: %s", config_option);
    }

    if (updated) {
        //ResizeDiagram();
    }
}

void ExtrusionDiagram::UpdateMinMax(const wxPoint& point){

    if (point.x < m_min_point.x) m_min_point.x = point.x;
    if (point.y < m_min_point.y) m_min_point.y = point.y;

    if (point.x > m_max_point.x) m_max_point.x = point.x;
    if (point.y > m_max_point.y) m_max_point.y = point.y;
}

void ExtrusionDiagram::InitializeMinMaxPoints() {

    m_min_point = wxPoint(INT_MAX, INT_MAX);
    m_max_point = wxPoint(INT_MIN, INT_MIN);
}