#include "CalibrationPressureAdvDialog.hpp"
#include "I18N.hpp"
#include "libslic3r/Utils.hpp"
#include "libslic3r/CustomGCode.hpp"
#include "libslic3r/Model.hpp"
#include "libslic3r/AppConfig.hpp"
#include "GLCanvas3D.hpp"
#include "GUI.hpp"
#include "GUI_ObjectList.hpp"
#include "Plater.hpp"
#include "Tab.hpp"
#include <wx/scrolwin.h>
#include <wx/display.h>
#include <wx/file.h>
#include "wxExtensions.hpp"
#include "Jobs/ArrangeJob.hpp"
#include <unordered_map>


#if ENABLE_SCROLLABLE
static wxSize get_screen_size(wxWindow* window)
{
    const auto idx = wxDisplay::GetFromWindow(window);
    wxDisplay display(idx != wxNOT_FOUND ? idx : 0u);
    return display.GetClientArea().GetSize();
}
#endif // ENABLE_SCROLLABLE

namespace Slic3r {
namespace GUI {

void CalibrationPressureAdvDialog::create_buttons(wxStdDialogButtonSizer* buttons){

    wxString choices_first_layerPA[] = {
        "0.025",
        "0.030",
        "0.035",
        "0.040",
        "0.045",
        "0.050"
    };
    firstPa = new wxComboBox(this, wxID_ANY, wxString{ "0.040" }, wxDefaultPosition, wxDefaultSize, 6, choices_first_layerPA);
    firstPa->SetToolTip(_L("Select the first layer PA value to be used for the first layer only."));
    firstPa->SetSelection(3);// starting at 0!


    wxString choices_start_PA[] = {
        "0.0",
        "0.010",
        "0.020",
        "0.030",
        "0.040",
        "0.050"
    };
    startPa = new wxComboBox(this, wxID_ANY, wxString{ "0.0" }, wxDefaultPosition, wxDefaultSize, 6, choices_start_PA);
    startPa->SetToolTip(_L("Select the starting PA value to be used."));
    startPa->SetSelection(0);

    wxString choices_end_PA[] = {
        "0.10",
        "0.20",
        "0.30",
        "0.40",
        "0.50",
        "0.60",
        "0.70",
        "0.80",
        "0.90",
        "1.00"
    };
    endPa = new wxComboBox(this, wxID_ANY, wxString{ "0.10" }, wxDefaultPosition, wxDefaultSize, 10, choices_end_PA);
    endPa->SetToolTip(_L("Select the ending PA value to be used."));
    endPa->SetSelection(0);

    wxString choices_increment_PA[] = {
        "0.0010",///1000 hits
        "0.0025",
        "0.005", //200 hits
        "0.006",
        "0.007",
        "0.01",//100 hits
        "0.1"//10 hits
    };
    paIncrement = new wxComboBox(this, wxID_ANY, wxString{ "0.0025" }, wxDefaultPosition, wxDefaultSize, 7, choices_increment_PA);
    paIncrement->SetToolTip(_L("Select the PA increment amount."));
    paIncrement->SetSelection(1);

    wxString choices_extrusion_role[] = {
    "InternalInfill",
    "BridgeInfill",
    "ExternalPerimeter",
    "GapFill",
    "InternalBridgeInfill",
    "Ironing",
    "OverhangPerimeter",
    "Perimeter",
    "SolidInfill",
    "SupportMaterial",
    "SupportMaterialInterface",
    "ThinWall",
    "TopSolidInfill"
    };
    erPa = new wxComboBox(this, wxID_ANY, wxString{ "InternalInfill" }, wxDefaultPosition, wxDefaultSize, 13, choices_extrusion_role);
    erPa->SetToolTip(_L("Select the extrusion role you want to generate a calibration for"));
    erPa->SetSelection(0);


    wxString number_of_runs[] = {"1","2","3","4","5"};
    nbRuns = new wxComboBox(this, wxID_ANY, wxString{ "1" }, wxDefaultPosition, wxDefaultSize, 5, number_of_runs);
    nbRuns->SetToolTip(_L("Select the number of tests to generate, max 2 is reccomended due to bed size limits"));
    nbRuns->SetSelection(0);
    

    // TODO : add another row of boxes for the 2nd/3rd ect of tests to create, user adjust parameters of new row for the 2nd/3rd test
    //      this will allow multi plate PA tests to be run

    
    buttons->Add(new wxStaticText(this, wxID_ANY, _L("Number of tests to create: ")));
    buttons->Add(nbRuns);
    buttons->AddSpacer(15);
    buttons->Add(new wxStaticText(this, wxID_ANY, _L("First Layers PA value: ")));
    buttons->Add(firstPa);
    buttons->AddSpacer(15);
    buttons->Add(new wxStaticText(this, wxID_ANY, _L("Starting PA value: ")));
    buttons->Add(startPa);
    buttons->AddSpacer(15);
    buttons->Add(new wxStaticText(this, wxID_ANY, _L("Ending PA value: ")));
    buttons->Add(endPa);
    buttons->AddSpacer(15);
    buttons->Add(new wxStaticText(this, wxID_ANY, _L("PA increments: ")));
    buttons->Add(paIncrement);
    buttons->AddSpacer(15);
    buttons->Add(new wxStaticText(this, wxID_ANY, _L("Extrusion role: ")));
    buttons->Add(erPa);
    buttons->AddSpacer(25);


    wxButton* bt = new wxButton(this, wxID_FILE1, _L("Generate"));
    bt->Bind(wxEVT_BUTTON, &CalibrationPressureAdvDialog::create_geometry, this);
    buttons->Add(bt);
    //this->CenterOnParent();
}

void CalibrationPressureAdvDialog::create_geometry(wxCommandEvent& event_args) {
    /*
    firstPa
    startPa
    endPa
    paIncrement
    erPa
    */
    double first_pa, start_pa, end_pa, pa_increment = 0.01;

    first_pa = firstPa->GetValue().ToDouble(&first_pa);
    if (!firstPa->GetValue().ToDouble(&first_pa)) {
        first_pa = 0.025;
    }
    start_pa = startPa->GetValue().ToDouble(&start_pa);
    if (!startPa->GetValue().ToDouble(&start_pa)) {
        start_pa = 0.0;
    }
    end_pa = endPa->GetValue().ToDouble(&end_pa);
    if (!endPa->GetValue().ToDouble(&end_pa)) {
        end_pa = 1.0;
    }
    pa_increment = paIncrement->GetValue().ToDouble(&pa_increment);
    if (!paIncrement->GetValue().ToDouble(&pa_increment)) {
        pa_increment = 0.05;
    }
    size_t nb_runs = 1;
    if (nbRuns->GetSelection() == 1) {
        nb_runs = 2;
    } else if (nbRuns->GetSelection() == 2) {
        nb_runs = 3;
    } else if (nbRuns->GetSelection() == 3) {
        nb_runs = 4;
    } else if (nbRuns->GetSelection() == 4) {
        nb_runs = 5;
    }
    else{
        nb_runs = 1;
    }

    std::string extrusion_role = erPa->GetValue().ToStdString();
    std::string  choice_extrusion_role[] = {
    "InternalInfill",
    "BridgeInfill",
    "ExternalPerimeter",
    "GapFill",
    "InternalBridgeInfill",
    "Ironing",
    "OverhangPerimeter",
    "Perimeter",
    "SolidInfill",
    "SupportMaterial",
    "SupportMaterialInterface",
    "ThinWall",
    "TopSolidInfill"
    };

   std::unordered_map<std::string, std::string> er_width_ToOptionKey = {
    {"InternalInfill", "infill_extrusion_width"},
    //{"BridgeInfill", "placeholder"},//special calc required
    {"ExternalPerimeter", "external_perimeter_extrusion_width"},
    //{"GapFill", "placeholder"},//special calc required
    //{"InternalBridgeInfill", "placeholder"},//special calc required, not sure how to process these for the width since they're not a static value
    {"Ironing", "top_infill_extrusion_width"},
    {"OverhangPerimeter", "overhangs_width"},
    {"Perimeter", "perimeter_extrusion_width"},
    {"SolidInfill", "solid_infill_extrusion_width"},
    {"SupportMaterial", "support_material_extrusion_width"},
    {"SupportMaterialInterface", "support_material_extrusion_width"},
    {"ThinWall", "external_perimeter_extrusion_width"},
    {"TopSolidInfill", "top_infill_extrusion_width"}
    };

    std::unordered_map<std::string, std::string> er_accel_ToOptionKey = {
    {"InternalInfill", "infill_acceleration"},
    {"BridgeInfill", "bridge_acceleration"},
    {"ExternalPerimeter", "external_perimeter_acceleration"},
    {"GapFill", "gap_fill_acceleration"},
    {"InternalBridgeInfill", "bridge_internal_acceleration"},
    {"Ironing", "ironing_acceleration"},
    {"OverhangPerimeter", "overhangs_acceleration"},
    {"Perimeter", "perimeter_acceleration"},
    {"SolidInfill", "solid_infill_acceleration"},
    {"SupportMaterial", "support_material_acceleration"},
    {"SupportMaterialInterface", "support_material_interface_acceleration"},
    {"ThinWall", "top_solid_infill_acceleration"},
    {"TopSolidInfill", "top_solid_infill_acceleration"}
    };

    std::unordered_map<std::string, std::string> er_speed_ToOptionKey = {
    {"InternalInfill", "infill_speed"},
    {"BridgeInfill", "bridge_speed"},
    {"ExternalPerimeter", "external_perimeter_speed"},
    {"GapFill", "gap_fill_speed"},
    {"InternalBridgeInfill", "bridge_speed_internal"},
    {"Ironing", "ironing_speed"},
    {"OverhangPerimeter", "overhangs_speed"},
    {"Perimeter", "perimeter_speed"},
    {"SolidInfill", "solid_infill_speed"},
    {"SupportMaterial", "support_material_speed"},
    {"SupportMaterialInterface", "support_material_interface_speed"},
    {"ThinWall", "thin_walls_speed"},
    {"TopSolidInfill", "top_solid_infill_speed"}
    };

/*
struct ExtrusionSettings {// think a map is better?
    std::string extrusionWidth;
    std::string acceleration;
    std::string speed;
};

    std::unordered_map<std::string, ExtrusionSettings> extrusionRoleToOptionKey = {
        {"InternalInfill", {"infill_extrusion_width", "infill_acceleration", "placeholder"}},
        //{"BridgeInfill", {"placeholder", "bridge_acceleration", "placeholder"}},//special calc required
        {"ExternalPerimeter", {"external_perimeter_extrusion_width", "external_perimeter_acceleration"}},
        //{"GapFill", {"placeholder", "gap_fill_acceleration"}},//special calc required
        //{"InternalBridgeInfill", {"placeholder", "bridge_internal_acceleration"}},//special calc required
        {"Ironing", {"top_infill_extrusion_width", "ironing_acceleration"}},
        {"OverhangPerimeter", {"overhangs_width", "overhangs_acceleration"}},
        {"Perimeter", {"perimeter_extrusion_width", "perimeter_acceleration"}},
        {"SolidInfill", {"solid_infill_extrusion_width", "solid_infill_acceleration"}},
        {"SupportMaterial", {"support_material_extrusion_width", "support_material_acceleration"}},
        {"SupportMaterialInterface", {"support_material_extrusion_width", "support_material_interface_acceleration"}},
        {"ThinWall", {"external_perimeter_extrusion_width", "thin_walls_acceleration"}},
        {"TopSolidInfill", {"top_infill_extrusion_width", "top_solid_infill_acceleration"}}
    };*/
    
    int countincrements = 0;
    int sizeofarray = static_cast<int>((end_pa - start_pa) / pa_increment) + 1;
    std::vector<double> pa_values(sizeofarray);
    std::vector<std::string> c_pa_values_c(sizeofarray);
    
    double incremented_pa_value = start_pa;
    while (incremented_pa_value <= end_pa + pa_increment / 2) {//this makes a number to be used to load x number of 90 bend models for the PA test.

        double rounded_Pa = std::round(incremented_pa_value * 100000.0) / 100000.0;
        pa_values[countincrements] = rounded_Pa;//store PA numbers in array to be used later. // this might be where it's not getting the 0.1 number ?
        c_pa_values_c[countincrements] = rounded_Pa;
        countincrements++;
        incremented_pa_value += pa_increment;
        // is there a limit of how many models SS can load ? might be good to set a failsafe just so it won't load 10k+ models...
    }

    bool has_to_arrange = false;
    Plater* plat = this->main_frame->plater();
    Model& model = plat->model();
    if (!plat->new_project(L("Pressure calibration")))
        return;

    bool autocenter = gui_app->app_config->get("autocenter") == "1";
    if (autocenter) {
        //disable auto-center for this calibration.
        gui_app->app_config->set("autocenter", "0");
    }
    
    std::vector<std::string> items;
    for (size_t i = 0; i < nb_runs; i++){
        items.emplace_back((boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "base_plate.3mf").string());
    }
    std::vector<size_t> objs_idx = plat->load_files(items, true, false, false, false);
    assert(objs_idx.size() == nb_runs);
    const DynamicPrintConfig* print_config = this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->get_config();
    const DynamicPrintConfig* filament_config = this->gui_app->get_tab(Preset::TYPE_FFF_FILAMENT)->get_config();
    const DynamicPrintConfig* printer_config = this->gui_app->get_tab(Preset::TYPE_PRINTER)->get_config();
    
    // --- scale ---
    //model is created for a 0.4 nozzle, scale xy with nozzle size.
    // NOTE: can't stretch 90 bend models since that will mess with part positioning and it won't have it's perfect 90° bend anymore
    const ConfigOptionFloats* nozzle_diameter_config = printer_config->option<ConfigOptionFloats>("nozzle_diameter");
    assert(nozzle_diameter_config->values.size() > 0);
    float nozzle_diameter = nozzle_diameter_config->values[0];
    float xyzScale = nozzle_diameter / 0.4;
    double first_layer_height = print_config->get_abs_value("first_layer_height", nozzle_diameter);
    GCodeFlavor flavor = printer_config->option<ConfigOptionEnum<GCodeFlavor>>("gcode_flavor")->value;
    
    //get ER widths, ER accell, ER speeds
    double er_width = print_config->get_abs_value("solid_infill_extrusion_width", nozzle_diameter);
    double er_accel = print_config->get_abs_value("solid_infill_acceleration", nozzle_diameter);
    double er_speed = print_config->get_abs_value("solid_infill_speed", nozzle_diameter);

    double default_er_width = print_config->get_abs_value("extrusion_width", nozzle_diameter);
    double default_er_accel = print_config->get_abs_value("default_acceleration", nozzle_diameter);
    double default_er_speed = print_config->get_abs_value("default_speed", nozzle_diameter);


    for (int i = 0; i < sizeof(choice_extrusion_role) / sizeof(choice_extrusion_role[0]); i++) {
        
        if (er_width_ToOptionKey.find(extrusion_role) != er_width_ToOptionKey.end()) {

            er_width = print_config->get_abs_value(er_width_ToOptionKey[extrusion_role].c_str(), nozzle_diameter);//look at maps at match speed/width ect to the selecter ER role
            er_speed = print_config->get_abs_value(er_speed_ToOptionKey[extrusion_role].c_str(), nozzle_diameter);
            er_accel = print_config->get_abs_value(er_accel_ToOptionKey[extrusion_role].c_str(), nozzle_diameter);

            //potential BUG if any of the values are 0 everything else would fail, need to pull the default value too and assign that?
            
            er_width = er_width * 100 / nozzle_diameter;
            er_width = std::round(er_width * 100.0) / 100.0;  // Change number to percentage and round
        } else {
            er_width = print_config->get_abs_value("solid_infill_extrusion_width", nozzle_diameter); //used for gapfill_width/bridges selection.
            er_width = er_width * 100 / nozzle_diameter;
            er_width = std::round(er_width * 100.0) / 100.0;  // Change number to percentage and round

        }
    
    }

    //base scale for model/parts
    if (xyzScale < 0.9 || 1.1 < xyzScale) {
        model.objects[objs_idx[0]]->scale(xyzScale, xyzScale * 0.5, xyzScale);
    } else {
        xyzScale = 1;
        model.objects[objs_idx[0]]->scale(xyzScale, xyzScale * 0.5, xyzScale);
    }

    float zshift = (1 - xyzScale) / 2;
    std::vector < std::vector<ModelObject*>> pressure_tower;

    for (size_t id_item = 0; id_item < nb_runs; id_item++) {
        pressure_tower.emplace_back();
        double xpos = 22 * xyzScale;
        double ypos = 22 * xyzScale;
        double xpos_stretch = 1.5 * xyzScale;
        double ypos_stretch = 2.5 * xyzScale;
        double x_total = xpos*countincrements;
        double y_total = ypos*countincrements;
        double initial_model_height = 0.2;
        double z_scale_factor = first_layer_height / initial_model_height; // BUG: output error if first layer height is lower than base layer height?
                                                                            //      this can cause the numbers to not "show up" on the preview because the z scale is calculated wrong.
                                                                            // ie; first_layer_height=0.1 and base_layer_height =0.20

        double new_z_world_coords = first_layer_height / 2.0 -0.10;//"-0.10" needed for some reason, otherwise it messes up the positioning of the models
        double new_num_z_world_coords = first_layer_height / 2.0 + new_z_world_coords;

        for (int nb_bends = 0; nb_bends <= countincrements -1; nb_bends++){
            pressure_tower.back().push_back(add_part(model.objects[objs_idx[id_item]], 
                    (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "90_bend_v2.3mf").string(),
                    Vec3d{ 0, nb_bends * 5 * xyzScale, 0.5 }, Vec3d{ xyzScale, xyzScale/**1.43*/, xyzScale*6.0}));
        }


        for (int nb_bends = 0; nb_bends <= countincrements;nb_bends++){
            if(nb_bends == countincrements / 2 ) {
                add_part(model.objects[objs_idx[id_item]], (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "pa_border_s.3mf").string(),
                    Vec3d{ 1.4 , nb_bends * 5 * xyzScale, new_z_world_coords }, Vec3d{ xyzScale*1.43, (nb_bends+0.5) * xyzScale / 2, z_scale_factor });// left sides border
            }
            if(nb_bends == countincrements / 2 ) {
                add_part(model.objects[objs_idx[id_item]], (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "pa_border_s.3mf").string(),
                    Vec3d{ 51.5 , nb_bends * 5 * xyzScale, new_z_world_coords }, Vec3d{ xyzScale *14, (nb_bends+0.5) * xyzScale / 2, z_scale_factor });// right sides border
            }
        }

        for (int nb_bends = 0; nb_bends < countincrements;nb_bends++){
            std::string pa_values_string = std::to_string(pa_values[nb_bends]);
            std::string threemf =".3mf";
            xpos = 22 * xyzScale;//reset x coords for numbers position.
            int pa_number = pa_values[nb_bends];
            if(nb_bends == 0) {
                add_part(model.objects[objs_idx[id_item]], (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "pa_border_b.3mf").string(),
                    Vec3d{ 10.9 , nb_bends -1.5 , new_z_world_coords }, Vec3d{ 1 * 1.58 *xyzScale, xyzScale+1, z_scale_factor });//bottom border
            }if(nb_bends == countincrements -1) {
                add_part(model.objects[objs_idx[id_item]], (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "pa_border_b.3mf").string(),
                    Vec3d{ 10.9 , ypos +1.0 , new_z_world_coords }, Vec3d{ 1 * 1.58 *xyzScale, xyzScale+1, z_scale_factor });//top border
            }
        
        
            for (int j = 0; j < 7; ++j) { // loop though new array starting from begining, not sure how the code will respond with a positive array list? ie ; 100.2 this moves decimal point thus breaking the code from loading model since "..3mf" not a real file...

                std::string numered3mfpath = pa_values_string[j] + threemf;
                
                if (pa_values_string[j] == '.') {

                    add_part(model.objects[objs_idx[id_item]], (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / "point.3mf").string(),
                        Vec3d{ xpos , ypos -3.6 , new_num_z_world_coords }, Vec3d{ xyzScale, xyzScale+1.5, z_scale_factor });
                    xpos = xpos+1.5;
                }
                else if (std::isdigit(pa_values_string[j])) {


                    add_part(model.objects[objs_idx[id_item]], (boost::filesystem::path(Slic3r::resources_dir()) / "calibration" / "filament_pressure" / numered3mfpath).string(),
                        Vec3d{ xpos , ypos , new_num_z_world_coords }, Vec3d{ xyzScale, xyzScale, z_scale_factor });
                    //  position in printer                          //scale model
                    xpos = xpos+2.8;
                }
            }
        
            ypos = ypos+5;//might need to change this for larger tests?
        }
        x_total = xpos*countincrements;
        y_total = ypos;
        //i++;
    }


    /// --- translate ---
    //bool autocenter = gui_app->app_config->get("autocenter") == "1";
    has_to_arrange = true;
    /*if (!autocenter) {
        const ConfigOptionPoints* bed_shape = printer_config->option<ConfigOptionPoints>("bed_shape");
        Vec2d bed_size = BoundingBoxf(bed_shape->values).size();
        Vec2d bed_min = BoundingBoxf(bed_shape->values).min;
        model.objects[objs_idx[0]]->translate({ bed_min.x() + bed_size.x() / 2, bed_min.y() + bed_size.y() / 2, 5 * xyzScale - 5 });
    }*/

    /// --- main config, modify object config when possible ---
    DynamicPrintConfig new_print_config = *print_config;
    new_print_config.set_key_value("complete_objects", new ConfigOptionBool(true)); //should be false?, check later
    //new_print_config.set_key_value("gap_fill_enabled", new ConfigOptionBool(false));
    new_print_config.set_key_value("top_solid_layers", new ConfigOptionInt(0));  //BUG: top layers set to 0 the top layer has a "void" where the top layer would normally be, this might be a config thing that needs to get changed or a slicing bug
    new_print_config.set_key_value("bottom_solid_layers", new ConfigOptionInt(1));
    new_print_config.set_key_value("fill_density", new ConfigOptionPercent(0));
    new_print_config.set_key_value("min_width_top_surface", new ConfigOptionFloatOrPercent(0.0,false));
    new_print_config.set_key_value("avoid_crossing_perimeters", new ConfigOptionBool(false));
    new_print_config.set_key_value("per_objects_gcode", new ConfigOptionString("90_bend_v2.3mf,"));// this is the model other parts in code will search for and insert the PA/ect numbers

    for (int16_t i = 1; i < countincrements; i++) {
        /*
        gcfRepRap,
        gcfSprinter,
        gcfRepetier,
        gcfTeacup,
        gcfMakerWare,
        gcfMarlinLegacy,
        gcfMarlinFirmware,
        gcfLerdge,
        gcfKlipper,
        gcfSailfish,
        gcfMach3,
        gcfMachinekit,
        gcfSmoothie,
        gcfNoExtrusion*/


        if (gcfKlipper == flavor) {// gcfKlipper

            //thought about adding in the lines here, but then thought of "per_objects_gcode"
            //i don't like how the custom_gcode makes the gcode preview a solid color, makes it hard to view/ check things.
        }
        else if (gcfMarlinFirmware == flavor) {
        }
        else if (gcfSprinter == flavor) {
        }
        else if (gcfRepetier == flavor) {
        }
        else if (gcfTeacup == flavor) {
        }
        else if (gcfMakerWare == flavor) {
        }


    }

    /// --- custom config --- // this part is for forcing each model to have x print modifiers
    for (size_t i = 0; i < nb_runs; i++) {
        size_t num_part = 0;
        const int extra_vol = 1;
        for (ModelObject* part : pressure_tower[i]) {//loop though each part/volume and assign the modifers

            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("perimeter_extrusion_width", new ConfigOptionFloatOrPercent(er_width, true));
            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("external_perimeter_extrusion_width", new ConfigOptionFloatOrPercent(er_width, true));

            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("perimeter_speed", new ConfigOptionFloatOrPercent(er_speed, false));
            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("external_perimeter_speed", new ConfigOptionFloatOrPercent(er_speed, false));
            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("gap_fill_speed", new ConfigOptionFloatOrPercent(er_speed, false));

            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("perimeter_acceleration", new ConfigOptionFloatOrPercent(er_accel, false));
            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("external_perimeter_acceleration", new ConfigOptionFloatOrPercent(er_accel, false));
            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("gap_fill_acceleration", new ConfigOptionFloatOrPercent(er_accel, false));
            //model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("between_objects_gcode", new ConfigOptionString("SET_PRESSURE_ADVANCE ADVANCE=" + std::to_string(pa_values[num_part]))); // "between_objects_gcode" has validation and ect, figured it was easier to make a new thing.
            model.objects[objs_idx[i]]->volumes[num_part + extra_vol]->config.set_key_value("per_objects_gcode", new ConfigOptionString("SET_PRESSURE_ADVANCE ADVANCE=" + std::to_string(pa_values[num_part])));

            num_part++;
        }
    }

    //update plater
    this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->load_config(new_print_config);
    plat->on_config_change(new_print_config);
    //this->gui_app->get_tab(Preset::TYPE_PRINTER)->load_config(new_printer_config);
    //plat->on_config_change(new_printer_config);
    plat->changed_objects(objs_idx);
    this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->update_dirty();
    //this->gui_app->get_tab(Preset::TYPE_PRINTER)->update_dirty();
    plat->is_preview_shown();
    //update everything, easier to code.
    ObjectList* obj = this->gui_app->obj_list();
    obj->update_after_undo_redo();


    // arrange if needed, after new settings, to take them into account
    if (has_to_arrange) {
        //update print config (done at reslice but we need it here)
        if (plat->printer_technology() == ptFFF)
            plat->fff_print().apply(plat->model(), *plat->config());
        std::shared_ptr<ProgressIndicatorStub> fake_statusbar = std::make_shared<ProgressIndicatorStub>();
        ArrangeJob arranger(std::dynamic_pointer_cast<ProgressIndicator>(fake_statusbar), plat);
        arranger.prepare_all();
        arranger.process();
        arranger.finalize();
    }


    // need to add if statement to check if loaded model is within print bounds?
    plat->reslice(); //forces a slice of plater.

    if (autocenter) {
        //re-enable auto-center after this calibration.
        gui_app->app_config->set("autocenter", "1");
    }
}

} // namespace GUI
} // namespace Slic3r
