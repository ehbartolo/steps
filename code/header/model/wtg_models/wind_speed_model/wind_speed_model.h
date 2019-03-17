#ifndef WIND_SPEED_MODEL_H
#define WIND_SPEED_MODEL_H

#include "header/model/wtg_models/wtg_model.h"
class WIND_SPEED_MODEL : public WTG_MODEL
{
    public:
        WIND_SPEED_MODEL();
        virtual ~WIND_SPEED_MODEL();

        virtual string get_model_type() const;
        double get_nominal_wind_speed_in_mps() const;
    public: // specific model level
        virtual string get_model_name() const = 0;

        virtual bool setup_model_with_steps_string_vector(vector<string>& data) = 0;
        virtual bool setup_model_with_psse_string(string data) = 0;
        virtual bool setup_model_with_bpa_string(string data) = 0;

        virtual void initialize() = 0;
        virtual void run(DYNAMIC_MODE mode) = 0;
        double get_wind_speed_in_mps();
        virtual double get_wind_speed_in_pu() = 0;
        virtual double get_wind_direction_in_deg() = 0;

        virtual void check() = 0;
        virtual void clear() = 0;
        virtual void report() = 0;
        virtual void save() = 0;
        virtual string get_standard_model_string() const = 0;

        virtual void prepare_model_data_table() = 0;
        virtual double get_model_data_with_name(string par_name) const = 0;
        virtual void set_model_data_with_name(string par_name, double value) = 0;

        virtual void prepare_model_internal_variable_table() = 0;
        virtual double get_model_internal_variable_with_name(string var_name)= 0;

        virtual string get_dynamic_data_in_psse_format() const = 0;
        virtual string get_dynamic_data_in_bpa_format() const = 0;
        virtual string get_dynamic_data_in_steps_format() const = 0;
    private:

};

#endif // WIND_SPEED_MODEL_H
