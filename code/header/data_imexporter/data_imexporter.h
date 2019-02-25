#ifndef DATA_IMEXPORTER_H
#define DATA_IMEXPORTER_H

#include "header/power_system_database.h"
#include <string>
#include <fstream>

using namespace std;
class DATA_IMEXPORTER
{
    public:
        DATA_IMEXPORTER();
        virtual ~DATA_IMEXPORTER();

        void set_base_frequency_in_Hz(double fbase);
        void set_export_zero_impedance_line_logic(bool logic);
        double get_base_frequency_in_Hz() const;
        bool get_export_zero_impedance_line_logic() const;

        virtual void load_powerflow_data(string pf_source) = 0;
        virtual void load_dynamic_data(string dy_source) = 0;
        virtual void load_sequence_data(string sq_source) = 0;

        virtual void export_powerflow_data(string file, bool export_zero_impedance_line=true) = 0;
        virtual void export_dynamic_data(string file) = 0;
        virtual void export_sequence_data(string file) = 0;
    private:
        double base_frequency_in_Hz;
        bool export_zero_impedance_line;
};

#endif // DATA_IMEXPORTER_H
