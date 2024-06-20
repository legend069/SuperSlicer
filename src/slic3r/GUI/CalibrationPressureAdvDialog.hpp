#ifndef slic3r_GUI_CalibrationPressureAdvDialog_hpp_
#define slic3r_GUI_CalibrationPressureAdvDialog_hpp_

#include "CalibrationAbstractDialog.hpp"

namespace Slic3r {

	namespace GUI {


	class CalibrationPressureAdvDialog : public CalibrationAbstractDialog {
        public:
			CalibrationPressureAdvDialog(GUI_App* app, MainFrame* mainframe)
				: CalibrationAbstractDialog(app, mainframe, "Pressure Calibration")
			{
                create(boost::filesystem::path("calibration") / "filament_pressure", "filament_pressure.html",
                       wxSize(1600, 600));
                Centre(wxBOTH);
		}

			virtual ~CalibrationPressureAdvDialog() { }

		protected:
            void create_buttons(wxStdDialogButtonSizer *sizer) override;
            void create_geometry(wxCommandEvent &event_args);
            double      magical_scaling(double, double, double, double, double, double, double);

			wxComboBox* firstPa; //first layer PA 

            wxComboBox* startPa; // starting PA value

            wxComboBox *endPa; // ending PA value

            wxComboBox *paIncrement; // increment PA by this value

            wxComboBox *erPa; // extrusion role Pressure/Linear Advance 

            wxComboBox *nbRuns;

            wxCheckBox *enableST; // checkbox for smooth_time

	};

	}
}

#endif