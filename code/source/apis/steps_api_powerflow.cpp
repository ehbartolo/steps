#include "header/apis/steps_api.h"
#include "header/basic/utility.h"
#include "header/steps_namespace.h"

size_t api_get_powerflow_solver_integer_parameter(char* parameter_name, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();

    string PARAMETER_NAME = string2upper(parameter_name);
    if(PARAMETER_NAME=="MAX ITERATION")
        return solver.get_max_iteration();

    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Parameter '%s' is not supported for powerflow solver with api %s.\n"
             "0 will be returned.", PARAMETER_NAME.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return 0;
}

void api_set_powerflow_solver_integer_parameter(char* parameter_name, int value, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();

    string PARAMETER_NAME = string2upper(parameter_name);
    if(PARAMETER_NAME=="MAX ITERATION")
    {
        solver.set_max_iteration(value);
        return;
    }
    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Parameter '%s' is not supported for powerflow solver with api %s.\n",
             PARAMETER_NAME.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return;
}

double api_get_powerflow_solver_float_parameter(char* parameter_name, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    POWER_SYSTEM_DATABASE& psdb = toolkit.get_power_system_database();

    string PARAMETER_NAME = string2upper(parameter_name);
    if(PARAMETER_NAME=="MAX ACTIVE POWER IMBALANCE IN MW")
        return solver.get_allowed_max_active_power_imbalance_in_MW();
    if(PARAMETER_NAME=="MAX ACTIVE POWER IMBALANCE IN PU")
        return solver.get_allowed_max_active_power_imbalance_in_MW()/psdb.get_system_base_power_in_MVA();
    if(PARAMETER_NAME=="MAX REACTIVE POWER IMBALANCE IN MVAR")
        return solver.get_allowed_max_reactive_power_imbalance_in_MVar();
    if(PARAMETER_NAME=="MAX REACTIVE POWER IMBALANCE IN PU")
        return solver.get_allowed_max_reactive_power_imbalance_in_MVar()/psdb.get_system_base_power_in_MVA();
    if(PARAMETER_NAME=="ITERATION ACCELERATOR")
        return solver.get_iteration_accelerator();
    if(PARAMETER_NAME=="MAX VOLTAGE CHANGE IN PU")
        return solver.get_maximum_voltage_change_in_pu();
    if(PARAMETER_NAME=="MAX ANGLE CHANGE IN DEG")
        return solver.get_maximum_angle_change_in_deg();
    if(PARAMETER_NAME=="MAX ANGLE CHANGE IN RAD")
        return solver.get_maximum_angle_change_in_rad();

    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Parameter '%s' is not supported for powerflow solver with api %s.\n"
             "0 will be returned.", PARAMETER_NAME.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return 0.0;
}

void api_set_powerflow_solver_float_parameter(char* parameter_name, double value, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    POWER_SYSTEM_DATABASE& psdb = toolkit.get_power_system_database();

    string PARAMETER_NAME = string2upper(parameter_name);
    if(PARAMETER_NAME=="MAX ACTIVE POWER IMBALANCE IN MW")
    {
        solver.set_allowed_max_active_power_imbalance_in_MW(value);
        return;
    }
    if(PARAMETER_NAME=="MAX ACTIVE POWER IMBALANCE IN PU")
    {
        solver.set_allowed_max_active_power_imbalance_in_MW(value*psdb.get_system_base_power_in_MVA());
        return;
    }
    if(PARAMETER_NAME=="MAX REACTIVE POWER IMBALANCE IN MVAR")
    {
        solver.set_allowed_max_reactive_power_imbalance_in_MVar(value);
        return;
    }
    if(PARAMETER_NAME=="MAX REACTIVE POWER IMBALANCE IN PU")
    {
        solver.set_allowed_max_reactive_power_imbalance_in_MVar(value*psdb.get_system_base_power_in_MVA());
        return;
    }
    if(PARAMETER_NAME=="ITERATION ACCELERATOR")
    {
        solver.set_iteration_accelerator(value);
        return;
    }

    if(PARAMETER_NAME=="MAX VOLTAGE CHANGE IN PU")
    {
        solver.set_maximum_voltage_change_in_pu(value);
        return;
    }
    if(PARAMETER_NAME=="MAX ANGLE CHANGE IN DEG")
    {
        solver.set_maximum_angle_change_in_deg(value);
        return;
    }

    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Parameter '%s' is not supported for powerflow solver with api %s.\n",
             PARAMETER_NAME.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return;
}

bool api_get_powerflow_solver_boolean_parameter(char* parameter_name, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();

    string PARAMETER_NAME = string2upper(parameter_name);
    if(PARAMETER_NAME=="FLAT START LOGIC")
        return solver.get_flat_start_logic();
    if(PARAMETER_NAME=="TRANSFORMER TAP ADJUSTMENT LOGIC")
        return solver.get_transformer_tap_adjustment_logic();
    if(PARAMETER_NAME=="NON DIVERGENT SOLUTION LOGIC")
        return solver.get_non_divergent_solution_logic();
    if(PARAMETER_NAME=="EXPORT JACOBIAN LOGIC")
        return solver.get_export_jacobian_matrix_step_by_step_logic();

    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Parameter '%s' is not supported for powerflow solver with api %s.\n"
             "False will be returned.", PARAMETER_NAME.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return false;
}

void api_set_powerflow_solver_boolean_parameter(char* parameter_name, bool value, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();

    string PARAMETER_NAME = string2upper(parameter_name);
    if(PARAMETER_NAME=="FLAT START LOGIC")
    {
        solver.set_flat_start_logic(value);
        return;
    }
    if(PARAMETER_NAME=="TRANSFORMER TAP ADJUSTMENT LOGIC")
    {
        solver.set_transformer_tap_adjustment_logic(value);
        return;
    }
    if(PARAMETER_NAME=="NON DIVERGENT SOLUTION LOGIC")
    {
        solver.set_non_divergent_solution_logic(value);
        return;
    }
    if(PARAMETER_NAME=="EXPORT JACOBIAN LOGIC")
    {
        solver.set_export_jacobian_matrix_step_by_step_logic(value);
        return;
    }
    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Parameter '%s' is not supported for powerflow solver with api %s.\n",
             PARAMETER_NAME.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return;
}

void api_solve_powerflow(char* method, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();

    string string_method = string2upper(method);

    if(string_method == "NR")
    {
        solver.solve_with_full_Newton_Raphson_solution();
        return;
    }

    if(string_method == "PQ")
    {
        solver.solve_with_fast_decoupled_solution();
        return;
    }

    if(string_method == "GS")
    {
        solver.solve_with_modified_Gaussian_Seidel_solution();
        return;
    }

    char buffer[MAX_TEMP_CHAR_BUFFER_SIZE];
    snprintf(buffer, MAX_TEMP_CHAR_BUFFER_SIZE, "Method %s is not supported for solving powerflow with api %s.",
             string_method.c_str(), __FUNCTION__);
    toolkit.show_information_with_leading_time_stamp(buffer);
    return;
}

bool api_is_powerflow_converged(size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    return solver.is_converged();
}

void api_show_powerflow_result(size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    solver.show_powerflow_result();
}

void api_save_powerflow_result(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    solver.save_powerflow_result_to_file(file);
}

void api_save_extended_powerflow_result(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    solver.save_extended_powerflow_result_to_file(file);
}

void api_save_jacobian_matrix(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    solver.save_jacobian_matrix_to_file(file);
}

void api_build_network_matrix(size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.build_network_matrix();
}

void api_build_decoupled_network_matrix(size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.build_decoupled_network_matrix();
}

void api_build_dc_network_matrix(size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.build_dc_network_matrix();
}

void api_build_dynamic_network_matrix(size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.build_dynamic_network_matrix();
}

void api_save_network_matrix(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.save_network_matrix_to_file(file);
}

void api_save_decoupled_network_matrix(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.save_decoupled_network_matrix_to_file(file);
}

void api_save_dc_network_matrix(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.save_dc_network_matrix_to_file(file);
}

void api_save_dynamic_network_matrix(char* file, size_t toolkit_index)
{
    STEPS& toolkit = get_toolkit(toolkit_index);
    POWERFLOW_SOLVER& solver = toolkit.get_powerflow_solver();
    NETWORK_MATRIX& network_matrix = solver.get_network_matrix();
    network_matrix.save_dynamic_network_matrix_to_file(file);
}
