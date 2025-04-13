#ifndef slic3r_GUI_CalibrationPressureAdvDialog_hpp_
#define slic3r_GUI_CalibrationPressureAdvDialog_hpp_

#include "CalibrationAbstractDialog.hpp"
//pressure advance PressureAdv ss
namespace Slic3r { 
namespace GUI {

class CalibrationPressureAdvDialog : public CalibrationAbstractDialog
{

public:
    CalibrationPressureAdvDialog(GUI_App* app, MainFrame* mainframe) : CalibrationAbstractDialog(app, mainframe, "Pressure calibration") 
    { create(boost::filesystem::path("calibration") / "filament_pressure", "filament_pressure.html", wxSize(1600, 600), true); Centre(wxBOTH); currentTestCount = 1; } //include_close_button = true
    virtual ~CalibrationPressureAdvDialog(){ }

    void close_me_wrapper(wxCommandEvent& event);
    
protected:
    void create_buttons(wxStdDialogButtonSizer* sizer) override;
    void create_row_controls(wxBoxSizer* parent_sizer, int row_count);
    void create_geometry(wxCommandEvent& event_args);
    void on_row_change(wxCommandEvent& event);
    std::pair<std::vector<std::string>, int> calc_PA_values(int id_item);

    struct ScaledBend {
        double scale;// scale of the 90_bend model
        double first_layer_scale; // first layer scale of the 90_bend model 
        double perfect_sliced_width;// mm width of what the *4 extrusions should be
        double perfect_first_layer_width;// mm width of what the first layer *4 extrusions should be
    };
    ScaledBend magical_scaling(double, double, double, double, double, double, double, bool calc_first_layer = false );

    wxComboBox* nbRuns;

    std::vector<wxComboBox*> dynamicFirstPa;      //first layer PA -user manual entry
    std::vector<wxComboBox*> dynamicStartPa;      //starting PA value -user manual entry
    std::vector<wxComboBox*> dynamicEndPa;        //ending PA value -user manual entry
    std::vector<wxComboBox*> dynamicPaIncrement;  //increment PA by this value -user manual entry~~ or have drop down box ?
    std::vector<wxComboBox*> dynamicExtrusionRole;//extrusion role Pressure/Linear Advance -user choice select
    std::vector<wxCheckBox*> dynamicEnableST;     // checkbox for "smooth_time" - klipper only feature?
    std::vector<wxBoxSizer*> dynamicRowcount;     // To keep track of dynamically created rows

    wxBoxSizer* dynamicSizer;
    int currentTestCount;
};

} // namespace GUI
} // namespace Slic3r

#endif
