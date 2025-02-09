#include "header/network/network_matrix.h"
#include "header/basic/utility.h"
#include "header/STEPS.h"
#include <fstream>
#include <iostream>
#include <set>
using namespace std;

class SUBLINE
{
    public:
        SUBLINE()
        {
            from_loc = 0.0;
            to_loc = 1.0;
        }
        double from_loc;
        double to_loc;
};


NETWORK_MATRIX::NETWORK_MATRIX(STEPS& toolkit)
{
    this->toolkit = (&toolkit);
    clear();
}

NETWORK_MATRIX::~NETWORK_MATRIX()
{
    toolkit = NULL;
}

STEPS& NETWORK_MATRIX::get_toolkit() const
{
    return *toolkit;
}

void NETWORK_MATRIX::clear()
{
    network_Y1_matrix.clear();
    network_BP_matrix.clear();
    network_BQ_matrix.clear();
    network_DC_B_matrix.clear();

    network_Y1_matrix.clear();
    network_Y2_matrix.clear();
    network_Y0_matrix.clear();

    network_Z1_matrix.clear();
    network_Z2_matrix.clear();
    network_Z0_matrix.clear();
    inphno.clear();

    this_Y_matrix_pointer = NULL;
    this_Z_matrix_pointer = NULL;
    this_jacobian.clear();
}


void NETWORK_MATRIX::build_initial_zero_matrix(STEPS_COMPLEX_SPARSE_MATRIX& matrix)
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<BUS*> buses = psdb.get_all_buses();
    unsigned int nbus = buses.size();
    for(unsigned int i=0; i!=nbus; ++i)
    {
        BUS* bus = buses[i];
        if(bus->get_bus_type()!=OUT_OF_SERVICE)
        {
            unsigned int inbus = inphno.get_internal_bus_number_of_physical_bus_number(bus->get_bus_number());
            matrix.add_entry(inbus, inbus, 0);
        }
    }
}

void NETWORK_MATRIX::build_initial_zero_matrix(STEPS_SPARSE_MATRIX& matrix)
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<BUS*> buses = psdb.get_all_buses();
    unsigned int nbus = buses.size();
    for(unsigned int i=0; i!=nbus; ++i)
    {
        BUS* bus = buses[i];
        if(bus->get_bus_type()!=OUT_OF_SERVICE)
        {
            unsigned int inbus = inphno.get_internal_bus_number_of_physical_bus_number(bus->get_bus_number());
            matrix.add_entry(inbus, inbus, 0);
        }
    }
}

void NETWORK_MATRIX::build_network_Y_matrix()
{
    if(inphno.empty())
        initialize_physical_internal_bus_pair();

    network_Y1_matrix.clear();
    set_this_Y_and_Z_matrix_as(network_Y1_matrix);

    build_initial_zero_matrix(network_Y1_matrix);

    add_lines_to_network();
    add_transformers_to_network();
    add_fixed_shunts_to_network();

    network_Y1_matrix.compress_and_merge_duplicate_entries();
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_network_Y_matrix()
{
    if(network_Y1_matrix.matrix_in_triplet_form())
        build_network_Y_matrix();

    return network_Y1_matrix;
}

void NETWORK_MATRIX::build_decoupled_network_B_matrix()
{
    if(inphno.empty())
        initialize_physical_internal_bus_pair();

    network_BP_matrix.clear();
    network_BQ_matrix.clear();

    //build_initial_zero_matrix(network_BP_matrix);
    //build_initial_zero_matrix(network_BQ_matrix);

    add_lines_to_decoupled_network();
    add_transformers_to_decoupled_network();
    add_fixed_shunts_to_decoupled_network();

    network_BP_matrix.compress_and_merge_duplicate_entries();
    network_BQ_matrix.compress_and_merge_duplicate_entries();

}

STEPS_SPARSE_MATRIX& NETWORK_MATRIX::get_decoupled_network_BP_matrix()
{
    if(network_BP_matrix.matrix_in_triplet_form())
        build_decoupled_network_B_matrix();

    return network_BP_matrix;
}

STEPS_SPARSE_MATRIX& NETWORK_MATRIX::get_decoupled_network_BQ_matrix()
{
    if(network_BP_matrix.matrix_in_triplet_form())
        build_decoupled_network_B_matrix();

    return network_BQ_matrix;
}

void NETWORK_MATRIX::build_dc_network_B_matrix()
{
    if(inphno.empty())
        initialize_physical_internal_bus_pair();

    network_DC_B_matrix.clear();

    //build_initial_zero_matrix(network_DC_B_matrix);

    add_lines_to_dc_network();
    add_transformers_to_dc_network();

    network_DC_B_matrix.compress_and_merge_duplicate_entries();
}

STEPS_SPARSE_MATRIX& NETWORK_MATRIX::get_dc_network_B_matrix()
{
    if(network_DC_B_matrix.matrix_in_triplet_form())
        build_dc_network_B_matrix();

    return network_DC_B_matrix;
}


void NETWORK_MATRIX::build_dynamic_network_Y_matrix()
{
    if(inphno.empty())
        initialize_physical_internal_bus_pair();

    network_Y1_matrix.clear();
    set_this_Y_and_Z_matrix_as(network_Y1_matrix);

    //build_initial_zero_matrix(network_Y1_matrix);

    add_bus_fault_to_dynamic_network();
    add_lines_to_dynamic_network();
    add_transformers_to_network();
    add_fixed_shunts_to_network();
    add_generators_to_dynamic_network();
    add_wt_generators_to_dynamic_network();
    add_motor_loads_to_dynamic_network();
    add_vsc_hvdcs_to_dynamic_network();

    network_Y1_matrix.compress_and_merge_duplicate_entries();
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_dynamic_network_Y_matrix()
{
    if(network_Y1_matrix.matrix_in_triplet_form())
        build_dynamic_network_Y_matrix();

    return network_Y1_matrix;
}

void NETWORK_MATRIX::build_sequence_network_Y_matrix()
{
    build_positive_sequence_network_Y_matrix();
    build_negative_sequence_network_Y_matrix();
    build_zero_sequence_network_Y_matrix();
}

void NETWORK_MATRIX::build_positive_sequence_network_Y_matrix()
{
    build_dynamic_network_Y_matrix();
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_positive_sequence_network_Y_matrix()
{
    if(network_Y1_matrix.matrix_in_triplet_form())
        build_positive_sequence_network_Y_matrix();

    return network_Y1_matrix;
}

void NETWORK_MATRIX::build_negative_sequence_network_Y_matrix()
{
    ;
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_negative_sequence_network_Y_matrix()
{
    if(network_Y2_matrix.matrix_in_triplet_form())
        build_negative_sequence_network_Y_matrix();

    return network_Y2_matrix;
}

void NETWORK_MATRIX::build_zero_sequence_network_Y_matrix()
{
    ;
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_zero_sequence_network_Y_matrix()
{
    if(network_Y0_matrix.matrix_in_triplet_form())
        build_zero_sequence_network_Y_matrix();

    return network_Y0_matrix;
}

void NETWORK_MATRIX::build_network_Z_matrix_from_this_Y_matrix()
{
    build_this_jacobian_for_getting_impedance_from_this_Y_matrix();

    int n = this_Y_matrix_pointer->get_matrix_size();

    complex<double> temp(INFINITE_THRESHOLD,INFINITE_THRESHOLD);
    for(int i=0; i<n; ++i)
        for(int j=0; j<n; ++j)
            this_Z_matrix_pointer->add_entry(i,j,temp);
    this_Z_matrix_pointer->compress_and_merge_duplicate_entries();

    int n2 = n+n;
    for(int j=0; j<n; ++j)
    {
        vector<double> I;
        I.reserve(n2);
        for(int i=0; i<n2; ++i)
            I.push_back(0.0);
        I[j]=1.0;
        vector<double> U = I/this_jacobian;
        int k_start = this_Z_matrix_pointer->get_starting_index_of_column(j);
        for(int i=0; i<n; ++i)
        {
            complex<double> z(U[i], U[i+n]);
            if(z!=0.0)
                this_Z_matrix_pointer->change_entry_value(k_start+i, z);
        }
    }
}

void NETWORK_MATRIX::build_sequence_network_Z_matrix()
{
    build_positive_sequence_network_Z_matrix();
    build_negative_sequence_network_Z_matrix();
    build_zero_sequence_network_Z_matrix();
}

void NETWORK_MATRIX::build_positive_sequence_network_Z_matrix()
{
    set_this_Y_and_Z_matrix_as(network_Y1_matrix);
    network_Y1_matrix.report_brief();
    build_network_Z_matrix_from_this_Y_matrix();
}

void NETWORK_MATRIX::build_negative_sequence_network_Z_matrix()
{
    set_this_Y_and_Z_matrix_as(network_Y2_matrix);
    build_network_Z_matrix_from_this_Y_matrix();
}

void NETWORK_MATRIX::build_zero_sequence_network_Z_matrix()
{
    set_this_Y_and_Z_matrix_as(network_Y0_matrix);
    build_network_Z_matrix_from_this_Y_matrix();
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_positive_sequence_network_Z_matrix()
{
    if(network_Z1_matrix.get_matrix_size()<2)
        build_positive_sequence_network_Z_matrix();
    return network_Z1_matrix;
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_negative_sequence_network_Z_matrix()
{
    if(network_Z2_matrix.get_matrix_size()<2)
        build_negative_sequence_network_Z_matrix();
    return network_Z2_matrix;
}

STEPS_COMPLEX_SPARSE_MATRIX& NETWORK_MATRIX::get_zero_sequence_network_Z_matrix()
{
    if(network_Z0_matrix.get_matrix_size()<2)
        build_zero_sequence_network_Z_matrix();
    return network_Z0_matrix;
}

complex<double> NETWORK_MATRIX::get_positive_sequence_self_admittance_of_physical_bus(unsigned int bus)
{
    int b = get_internal_bus_number_of_physical_bus(bus);
    return network_Y1_matrix.get_entry_value(b, b);
}

complex<double> NETWORK_MATRIX::get_positive_sequence_mutual_admittance_between_physical_bus(unsigned int ibus, unsigned int jbus)
{
    int ib = get_internal_bus_number_of_physical_bus(ibus);
    int jb = get_internal_bus_number_of_physical_bus(jbus);
    return network_Y1_matrix.get_entry_value(ib, jb);
}

void NETWORK_MATRIX::set_this_Y_and_Z_matrix_as(STEPS_COMPLEX_SPARSE_MATRIX& matrix)
{
    this_Y_matrix_pointer = (& matrix);
    if(this_Y_matrix_pointer == &network_Y1_matrix)
        this_Z_matrix_pointer = &network_Z1_matrix;
    else
    {
        if(this_Y_matrix_pointer == &network_Y2_matrix)
            this_Z_matrix_pointer = &network_Z2_matrix;
        else
            this_Z_matrix_pointer = &network_Z0_matrix;
    }
}

void NETWORK_MATRIX::build_this_jacobian_for_getting_impedance_from_this_Y_matrix()
{
    this_jacobian.clear();
    int n = this_Y_matrix_pointer->get_matrix_size();
    for(int j=0; j<n; ++j)
    {
        int k_start = this_Y_matrix_pointer->get_starting_index_of_column(j);
        int k_end = this_Y_matrix_pointer->get_starting_index_of_column(j+1);
        for(int k=k_start; k<k_end; ++k)
        {
            int i = this_Y_matrix_pointer->get_row_number_of_entry_index(k);
            complex<double> y = this_Y_matrix_pointer->get_entry_value(k);
            double g = y.real(), b= y.imag();

            if(g!=0.0)
            {
                this_jacobian.add_entry(i,j,g);
                this_jacobian.add_entry(i+n,j+n,g);
            }
            if(b!=0.0)
            {
                this_jacobian.add_entry(i,  j+n, -b);
                this_jacobian.add_entry(i+n,j,   b);
            }
        }
    }
    this_jacobian.compress_and_merge_duplicate_entries();
    this_jacobian.LU_factorization(1, 1e-13);
}

vector<double> NETWORK_MATRIX::get_impedance_of_column_from_this_Y_matrix(unsigned int col)
{
    build_this_jacobian_for_getting_impedance_from_this_Y_matrix();

    int n = this_Y_matrix_pointer->get_matrix_size();
    int n2 = n+n;
    vector<double> I;
    I.reserve(n2);
    for(int i=0; i<n2; ++i)
        I.push_back(0.0);
    I[col]=1.0;
    vector<double> Z = I/this_jacobian;
    return Z;
}

vector<complex<double> > NETWORK_MATRIX::get_complex_impedance_of_column_from_this_Y_matrix(unsigned int col)
{
    build_this_jacobian_for_getting_impedance_from_this_Y_matrix();

    int n = this_Y_matrix_pointer->get_matrix_size();
    int n2 = n+n;
    vector<double> I;
    I.reserve(n2);
    for(int i=0; i<n2; ++i)
        I.push_back(0.0);
    I[col]=1.0;
    vector<double> Zreal = I/this_jacobian;
    vector<complex<double> > Z;
    Z.reserve(n);
    for(int i=0; i<n; ++i)
        Z.push_back(complex<double>(Zreal[i], Zreal[i+n]));
    return Z;
}

complex<double> NETWORK_MATRIX::get_self_impedance_of_physical_bus_from_this_Y_matrix(unsigned int bus)
{
    int bus_number = get_internal_bus_number_of_physical_bus(bus);

    /*vector<double> Z = get_impedance_of_column_from_this_Y_matrix(bus_number);
    int n = this_Y_matrix_pointer->get_matrix_size();
    complex<double> z(Z[bus_number], Z[bus_number+n]);*/
    vector<complex<double> > Z = get_complex_impedance_of_column_from_this_Y_matrix(bus_number);
    complex<double> z = Z[bus_number];
    return z;
}

complex<double> NETWORK_MATRIX::get_mutual_impedance_between_physical_bus_from_this_Y_matrix(unsigned int ibus, unsigned int jbus)
{
    int ibus_number = get_internal_bus_number_of_physical_bus(ibus);
    int jbus_number = get_internal_bus_number_of_physical_bus(jbus);

    /*vector<double> Z = get_impedance_of_column_from_this_Y_matrix(jbus_number);
    int n = this_Y_matrix_pointer->get_matrix_size();
    complex<double> z(Z[ibus_number], Z[ibus_number+n]);*/

    vector<complex<double> > Z = get_complex_impedance_of_column_from_this_Y_matrix(jbus_number);
    complex<double> z = Z[ibus_number];

    return z;
}

complex<double> NETWORK_MATRIX::get_positive_sequence_self_impedance_of_physical_bus(unsigned int bus)
{
    set_this_Y_and_Z_matrix_as(network_Y1_matrix);
    return get_self_impedance_of_physical_bus_from_this_Y_matrix(bus);
}

complex<double> NETWORK_MATRIX::get_positive_sequence_mutual_impedance_between_physical_bus(unsigned int ibus, unsigned int jbus)
{
    set_this_Y_and_Z_matrix_as(network_Y1_matrix);
    return get_mutual_impedance_between_physical_bus_from_this_Y_matrix(ibus, jbus);
}


complex<double> NETWORK_MATRIX::get_negative_sequence_self_admittance_of_physical_bus(unsigned int bus)
{
    int b = get_internal_bus_number_of_physical_bus(bus);
    return network_Y2_matrix.get_entry_value(b, b);
}

complex<double> NETWORK_MATRIX::get_negative_sequence_mutual_admittance_between_physical_bus(unsigned int ibus, unsigned int jbus)
{
    int ib = get_internal_bus_number_of_physical_bus(ibus);
    int jb = get_internal_bus_number_of_physical_bus(jbus);
    return network_Y2_matrix.get_entry_value(ib, jb);
}

complex<double> NETWORK_MATRIX::get_negative_sequence_self_impedance_of_physical_bus(unsigned int bus)
{
    set_this_Y_and_Z_matrix_as(network_Y2_matrix);
    return get_self_impedance_of_physical_bus_from_this_Y_matrix(bus);
}

complex<double> NETWORK_MATRIX::get_negative_sequence_mutual_impedance_between_physical_bus(unsigned int ibus, unsigned int jbus)
{
    set_this_Y_and_Z_matrix_as(network_Y2_matrix);
    return get_mutual_impedance_between_physical_bus_from_this_Y_matrix(ibus, jbus);
}

complex<double> NETWORK_MATRIX::get_zero_sequence_self_admittance_of_physical_bus(unsigned int bus)
{
    int b = get_internal_bus_number_of_physical_bus(bus);
    return network_Y0_matrix.get_entry_value(b, b);
}

complex<double> NETWORK_MATRIX::get_zero_sequence_mutual_admittance_between_physical_bus(unsigned int ibus, unsigned int jbus)
{
    set_this_Y_and_Z_matrix_as(network_Y0_matrix);

    int ib = get_internal_bus_number_of_physical_bus(ibus);
    int jb = get_internal_bus_number_of_physical_bus(jbus);
    return network_Y0_matrix.get_entry_value(ib, jb);
}

complex<double> NETWORK_MATRIX::get_zero_sequence_self_impedance_of_physical_bus(unsigned int bus)
{
    set_this_Y_and_Z_matrix_as(network_Y0_matrix);
    return get_self_impedance_of_physical_bus_from_this_Y_matrix(bus);
}

complex<double> NETWORK_MATRIX::get_zero_sequence_mutual_impedance_between_physical_bus(unsigned int ibus, unsigned int jbus)
{
    set_this_Y_and_Z_matrix_as(network_Y0_matrix);
    return get_mutual_impedance_between_physical_bus_from_this_Y_matrix(ibus, jbus);
}

void NETWORK_MATRIX::add_lines_to_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<LINE*> lines = psdb.get_all_lines();

    unsigned int n = lines.size();

    for(unsigned int i = 0; i!=n; ++i)
        add_line_to_network(*(lines[i]));
}


void NETWORK_MATRIX::add_line_to_network(const LINE& line)
{
    if(line.get_sending_side_breaker_status()==true or line.get_receiving_side_breaker_status()==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding line '%s' connecting to bus %u and %u to network Y matrix.", (line.get_identifier()).c_str(),
                      line.get_sending_side_bus(), line.get_receiving_side_bus());
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int sending_bus = line.get_sending_side_bus();
        unsigned int receiving_bus = line.get_receiving_side_bus();

        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(sending_bus);
        unsigned int j = inphno.get_internal_bus_number_of_physical_bus_number(receiving_bus);

        complex<double> Zline = line.get_line_positive_sequence_z_in_pu();
        complex<double> Yline = line.get_line_positive_sequence_y_in_pu();
        complex<double> Yshunt_sending = line.get_shunt_positive_sequence_y_at_sending_side_in_pu();
        complex<double> Yshunt_receiving = line.get_shunt_positive_sequence_y_at_receiving_side_in_pu();

        complex<double> Y, Z;
        if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==true)
        {
            Y = 1.0/Zline;

            this_Y_matrix_pointer->add_entry(i,j, -Y);
            this_Y_matrix_pointer->add_entry(j,i, -Y);
            this_Y_matrix_pointer->add_entry(i,i, Y+0.5*Yline+Yshunt_sending);
            this_Y_matrix_pointer->add_entry(j,j, Y+0.5*Yline+Yshunt_receiving);

            return;
        }

        if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==false)
        {
            Y = 0.5*Yline+Yshunt_receiving;
            Z = Zline + 1.0/Y;
            Y = 1.0/Z + 0.5*Yline+Yshunt_sending;
            this_Y_matrix_pointer->add_entry(i,i, Y);
            return;
        }

        if(line.get_sending_side_breaker_status()==false and line.get_receiving_side_breaker_status()==true)
        {
            Y = 0.5*Yline+Yshunt_sending;
            Z = Zline + 1.0/Y;
            Y = 1.0/Z + 0.5*Yline+Yshunt_receiving;
            this_Y_matrix_pointer->add_entry(j,j, Y);
            return;
        }
    }
}

void NETWORK_MATRIX::add_transformers_to_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<TRANSFORMER*> transformers = psdb.get_all_transformers();

    unsigned int n = transformers.size();

    for(unsigned int i = 0; i!=n; ++i)
        add_transformer_to_network(*(transformers[i]));
}

void NETWORK_MATRIX::add_transformer_to_network(const TRANSFORMER& trans)
{
    if(trans.is_two_winding_transformer())
        add_two_winding_transformer_to_network_v2(trans);
    else
        add_three_winding_transformer_to_network(trans);
}

void NETWORK_MATRIX::add_two_winding_transformer_to_network(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding two-winding transformer '%s' connecting to bus %u and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);
        complex<double> Ym = trans.get_magnetizing_admittance_based_on_winding_norminal_voltage_and_system_base_power_in_pu();

        complex<double> Zp, Zs;
        Zp = 0.5*Zps; Zs = Zp;

        double tap_primary = trans.get_winding_off_nominal_turn_ratio_in_pu(PRIMARY_SIDE);
        double angle_primary = trans.get_winding_angle_shift_in_deg(PRIMARY_SIDE);
        angle_primary = deg2rad(angle_primary);
        double tap_secondary = trans.get_winding_off_nominal_turn_ratio_in_pu(SECONDARY_SIDE);
        double angle_secondary = trans.get_winding_angle_shift_in_deg(SECONDARY_SIDE);
        angle_secondary = deg2rad(angle_secondary);

        complex<double> kp(tap_primary*steps_cos(angle_primary),tap_primary*steps_sin(angle_primary)),
                        ks(tap_secondary*steps_cos(angle_secondary),tap_secondary*steps_sin(angle_secondary));


        complex<double> V, I, Vstar;
        complex<double> ypp, yps, ysp, yss, Yeq, Zeq;

        complex<double> kp_store, ks_store, Ym_store;
        kp_store = kp;
        ks_store = ks;
        Ym_store = Ym;

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);
            this_Y_matrix_pointer->add_entry(p,s,yps);
            this_Y_matrix_pointer->add_entry(s,p,ysp);
            this_Y_matrix_pointer->add_entry(s,s,yss);


            kp = kp/ steps_fast_complex_abs(kp);
            ks = ks/ steps_fast_complex_abs(ks);
            Ym = 0.0;

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            network_BP_matrix.add_entry(p,p,ypp);
            network_BP_matrix.add_entry(p,s,yps);
            network_BP_matrix.add_entry(s,p,ysp);
            network_BP_matrix.add_entry(s,s,yss);


            kp = kp_store;
            ks = ks_store;

            kp = steps_fast_complex_abs(kp);
            ks = steps_fast_complex_abs(ks);
            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            network_BQ_matrix.add_entry(p,p,ypp);
            network_BQ_matrix.add_entry(p,s,yps);
            network_BQ_matrix.add_entry(s,p,ysp);
            network_BQ_matrix.add_entry(s,s,yss);


            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==false)
        {
            V = 1.0;
            V = V/kp;
            Zeq = Zp + 1.0/Ym;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);

            network_BP_matrix.add_entry(p,p,0.0);

            kp = steps_fast_complex_abs(kp);

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/kp;
            Zeq = Zp + 1.0/Ym;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            network_BQ_matrix.add_entry(p,p,ypp);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            V = 1.0;
            V = V/ks;
            Zeq = Zs + 1.0/Ym;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            this_Y_matrix_pointer->add_entry(s,s,yss);

            network_BP_matrix.add_entry(s,s,0.0);

            ks = steps_fast_complex_abs(ks);

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/ks;
            Zeq = Zs + 1.0/Ym;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            network_BQ_matrix.add_entry(s,s,yss);
            return;
        }
    }
}

void NETWORK_MATRIX::add_three_winding_transformer_to_network(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or
       trans.get_winding_breaker_status(SECONDARY_SIDE)==true or
       trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding three-winding transformer '%s' connecting to bus %u, %u, and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE), trans.get_winding_bus(TERTIARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);
        unsigned int tertiary_bus = trans.get_winding_bus(TERTIARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);
        unsigned int t = inphno.get_internal_bus_number_of_physical_bus_number(tertiary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);
        complex<double> Zst = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(SECONDARY_SIDE, TERTIARY_SIDE);
        complex<double> Zpt = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, TERTIARY_SIDE);
        complex<double> Ym = trans.get_magnetizing_admittance_based_on_winding_norminal_voltage_and_system_base_power_in_pu();

        complex<double> Zp = 0.5*(Zps+Zpt-Zst),
                        Zs = 0.5*(Zps+Zst-Zpt),
                        Zt = 0.5*(Zpt+Zst-Zps);

        double tap_primary = trans.get_winding_off_nominal_turn_ratio_in_pu(PRIMARY_SIDE);
        double angle_primary = trans.get_winding_angle_shift_in_deg(PRIMARY_SIDE);
        angle_primary = deg2rad(angle_primary);
        double tap_secondary = trans.get_winding_off_nominal_turn_ratio_in_pu(SECONDARY_SIDE);
        double angle_secondary = trans.get_winding_angle_shift_in_deg(SECONDARY_SIDE);
        angle_secondary = deg2rad(angle_secondary);
        double tap_tertiary = trans.get_winding_off_nominal_turn_ratio_in_pu(TERTIARY_SIDE);
        double angle_tertiary = trans.get_winding_angle_shift_in_deg(TERTIARY_SIDE);
        angle_tertiary = deg2rad(angle_tertiary);

        complex<double> kp(tap_primary*steps_cos(angle_primary),tap_primary*steps_sin(angle_primary)),
                        ks(tap_secondary*steps_cos(angle_secondary),tap_secondary*steps_sin(angle_secondary)),
                        kt(tap_tertiary*steps_cos(angle_tertiary),tap_tertiary*steps_sin(angle_tertiary));

        complex<double> V, I, Vstar;
        complex<double> ypp, yps, ypt, ysp, yss, yst, ytp, yts, ytt, Yeq, Zeq;

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            // voltage at primary side.
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            I = -Vstar/Zt;
            I /= conj(kt);
            ytp = I;

            // voltage at secondary side
            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp + 1.0/Zt;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            I = -Vstar/Zt;
            I /= conj(kt);
            yts = I;

            // voltage at tertiary side
            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zp + 1.0/Zs;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            ypt = I;

            I = -Vstar/Zs;
            I /= conj(ks);
            yst = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);
            this_Y_matrix_pointer->add_entry(p,s,yps);
            this_Y_matrix_pointer->add_entry(p,t,ypt);
            this_Y_matrix_pointer->add_entry(s,p,ysp);
            this_Y_matrix_pointer->add_entry(s,s,yss);
            this_Y_matrix_pointer->add_entry(s,t,yst);
            this_Y_matrix_pointer->add_entry(t,p,ytp);
            this_Y_matrix_pointer->add_entry(t,s,yts);
            this_Y_matrix_pointer->add_entry(t,t,ytt);
            return;
        }


        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);
            this_Y_matrix_pointer->add_entry(p,s,yps);
            this_Y_matrix_pointer->add_entry(s,p,ysp);
            this_Y_matrix_pointer->add_entry(s,s,yss);
            return;
        }


        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zt;
            I /= conj(kt);
            ytp = I;

            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            ypt = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);
            this_Y_matrix_pointer->add_entry(p,t,ypt);
            this_Y_matrix_pointer->add_entry(t,p,ytp);
            this_Y_matrix_pointer->add_entry(t,t,ytt);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zt;
            I /= conj(kt);
            yts = I;

            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            yst = I;

            this_Y_matrix_pointer->add_entry(s,s,yss);
            this_Y_matrix_pointer->add_entry(s,t,yst);
            this_Y_matrix_pointer->add_entry(t,s,yts);
            this_Y_matrix_pointer->add_entry(t,t,ytt);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            V = 1.0;
            V = V/kp;
            Zeq = Zp + 1.0/Ym;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            V = 1.0;
            V = V/ks;
            Zeq = Zs + 1.0/Ym;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            this_Y_matrix_pointer->add_entry(s,s,yss);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            V = 1.0;
            V = V/kt;
            Zeq = Zt + 1.0/Ym;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            this_Y_matrix_pointer->add_entry(t,t,ytt);
            return;
        }
    }
}

void NETWORK_MATRIX::add_two_winding_transformer_to_network_v2(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding two-winding transformer '%s' connecting to bus %u and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);
        complex<double> Ym = trans.get_magnetizing_admittance_based_on_winding_norminal_voltage_and_system_base_power_in_pu();

        complex<double> Zp, Zs;
        Zp = 0.5*Zps; Zs = Zp;

        double tap_primary = trans.get_winding_off_nominal_turn_ratio_in_pu(PRIMARY_SIDE);
        double angle_primary = trans.get_winding_angle_shift_in_deg(PRIMARY_SIDE);
        angle_primary = deg2rad(angle_primary);
        double tap_secondary = trans.get_winding_off_nominal_turn_ratio_in_pu(SECONDARY_SIDE);
        double angle_secondary = trans.get_winding_angle_shift_in_deg(SECONDARY_SIDE);
        angle_secondary = deg2rad(angle_secondary);

        complex<double> kp(tap_primary*steps_cos(angle_primary),tap_primary*steps_sin(angle_primary)),
                        ks(tap_secondary*steps_cos(angle_secondary),tap_secondary*steps_sin(angle_secondary));


        complex<double> V, I, Vstar;
        complex<double> ypp, yps, ysp, yss, Yeq, Zeq;

        complex<double> kp_store, ks_store, Ym_store;
        kp_store = kp;
        ks_store = ks;
        Ym_store = Ym;

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            Ym = 0.0;
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            this_Y_matrix_pointer->add_entry(p,p,ypp);
            this_Y_matrix_pointer->add_entry(p,s,yps);
            this_Y_matrix_pointer->add_entry(s,p,ysp);
            this_Y_matrix_pointer->add_entry(s,s,yss);

            Ym = Ym_store;
            this_Y_matrix_pointer->add_entry(p,p,Ym);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==false)
        {
            this_Y_matrix_pointer->add_entry(p,p,Ym);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            V = 1.0;
            V = V/ks;
            Ym = trans.get_magnetizing_admittance_based_on_winding_norminal_voltage_and_system_base_power_in_pu();
            Zeq = Zps + 1.0/Ym;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            this_Y_matrix_pointer->add_entry(s,s,yss);
            return;
        }
    }
}


void NETWORK_MATRIX::add_fixed_shunts_to_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<FIXED_SHUNT*> shunts = psdb.get_all_fixed_shunts();

    unsigned int n = shunts.size();

    for(unsigned int i = 0; i!=n; ++i)
        add_fixed_shunt_to_network(*(shunts[i]));
}

void NETWORK_MATRIX::add_fixed_shunt_to_network(const FIXED_SHUNT& shunt)
{
    if(shunt.get_status()==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding fixed shunt '%s' at bus %u to network Y matrix.", (shunt.get_identifier()).c_str(),
                      shunt.get_shunt_bus());
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int bus = shunt.get_shunt_bus();

        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(bus);

        complex<double> Yshunt = shunt.get_nominal_admittance_shunt_in_pu();

        this_Y_matrix_pointer->add_entry(i,i, Yshunt);
    }
}

void NETWORK_MATRIX::add_lines_to_decoupled_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<LINE*> lines = psdb.get_all_lines();

    unsigned int n = lines.size();

    for(unsigned int i = 0; i!=n; ++i)
    {
        add_line_to_decoupled_network(*(lines[i]));
    }
}

void NETWORK_MATRIX::add_line_to_decoupled_network(const LINE& line)
{
    if(line.get_sending_side_breaker_status()==true or line.get_receiving_side_breaker_status()==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding line '%s' connecting to bus %u and %u to network Y matrix.", (line.get_identifier()).c_str(),
                      line.get_sending_side_bus(), line.get_receiving_side_bus());
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int sending_bus = line.get_sending_side_bus();
        unsigned int receiving_bus = line.get_receiving_side_bus();

        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(sending_bus);
        unsigned int j = inphno.get_internal_bus_number_of_physical_bus_number(receiving_bus);

        complex<double> Zline = line.get_line_positive_sequence_z_in_pu();
        complex<double> Yline = line.get_line_positive_sequence_y_in_pu();
        complex<double> Yshunt_sending = line.get_shunt_positive_sequence_y_at_sending_side_in_pu();
        complex<double> Yshunt_receiving = line.get_shunt_positive_sequence_y_at_receiving_side_in_pu();

        complex<double> Y, Z;
        if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==true)
        {
            Y = 1.0/Zline;

            network_BP_matrix.add_entry(i,j, -Y);
            network_BP_matrix.add_entry(j,i, -Y);
            network_BP_matrix.add_entry(i,i, Y);
            network_BP_matrix.add_entry(j,j, Y);

            Zline = complex<double>(0.0, Zline.imag());
            Y = 1.0/Zline;
            Yline = complex<double>(0.0, Yline.imag());
            Yshunt_sending = complex<double>(0.0, Yshunt_sending.imag());
            Yshunt_receiving = complex<double>(0.0, Yshunt_receiving.imag());

            network_BQ_matrix.add_entry(i,j, -Y);
            network_BQ_matrix.add_entry(j,i, -Y);
            network_BQ_matrix.add_entry(i,i, Y+0.5*Yline+Yshunt_sending);
            network_BQ_matrix.add_entry(j,j, Y+0.5*Yline+Yshunt_receiving);

            return;
        }

        if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==false)
        {
            network_BP_matrix.add_entry(i,i, 0.0);

            Zline = complex<double>(0.0, Zline.imag());
            Yline = complex<double>(0.0, Yline.imag());
            Yshunt_sending = complex<double>(0.0, Yshunt_sending.imag());
            Yshunt_receiving = complex<double>(0.0, Yshunt_receiving.imag());

            Y = 0.5*Yline+Yshunt_receiving;
            Z = Zline + 1.0/Y;
            Y = 1.0/Z + 0.5*Yline+Yshunt_sending;

            network_BQ_matrix.add_entry(i,i, Y);
            return;
        }

        if(line.get_sending_side_breaker_status()==false and line.get_receiving_side_breaker_status()==true)
        {
            network_BP_matrix.add_entry(j,j, 0.0);

            Zline = complex<double>(0.0, Zline.imag());
            Yline = complex<double>(0.0, Yline.imag());
            Yshunt_sending = complex<double>(0.0, Yshunt_sending.imag());
            Yshunt_receiving = complex<double>(0.0, Yshunt_receiving.imag());

            Y = 0.5*Yline+Yshunt_sending;
            Z = Zline + 1.0/Y;
            Y = 1.0/Z + 0.5*Yline+Yshunt_receiving;
            this_Y_matrix_pointer->add_entry(j,j, Y);

            network_BQ_matrix.add_entry(j,j, Y);
            return;
        }
    }
}


void NETWORK_MATRIX::add_transformers_to_decoupled_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<TRANSFORMER*> transformers = psdb.get_all_transformers();

    unsigned int n = transformers.size();

    for(unsigned int i = 0; i!=n; ++i)
    {
        add_transformer_to_decoupled_network(*(transformers[i]));
    }
}

void NETWORK_MATRIX::add_transformer_to_decoupled_network(const TRANSFORMER& trans)
{
    if(trans.is_two_winding_transformer())
        add_two_winding_transformer_to_decoupled_network_v2(trans);
    else
        add_three_winding_transformer_to_decoupled_network(trans);
}

void NETWORK_MATRIX::add_three_winding_transformer_to_decoupled_network(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or
       trans.get_winding_breaker_status(SECONDARY_SIDE)==true or
       trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding three-winding transformer '%s' connecting to bus %u, %u, and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE), trans.get_winding_bus(TERTIARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);
        unsigned int tertiary_bus = trans.get_winding_bus(TERTIARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);
        unsigned int t = inphno.get_internal_bus_number_of_physical_bus_number(tertiary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);
        complex<double> Zst = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(SECONDARY_SIDE, TERTIARY_SIDE);
        complex<double> Zpt = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, TERTIARY_SIDE);
        complex<double> Ym = trans.get_magnetizing_admittance_based_on_winding_norminal_voltage_and_system_base_power_in_pu();

        complex<double> Zp = 0.5*(Zps+Zpt-Zst),
                        Zs = 0.5*(Zps+Zst-Zpt),
                        Zt = 0.5*(Zpt+Zst-Zps);

        double tap_primary = trans.get_winding_off_nominal_turn_ratio_in_pu(PRIMARY_SIDE);
        double angle_primary = trans.get_winding_angle_shift_in_deg(PRIMARY_SIDE);
        angle_primary = deg2rad(angle_primary);
        double tap_secondary = trans.get_winding_off_nominal_turn_ratio_in_pu(SECONDARY_SIDE);
        double angle_secondary = trans.get_winding_angle_shift_in_deg(SECONDARY_SIDE);
        angle_secondary = deg2rad(angle_secondary);
        double tap_tertiary = trans.get_winding_off_nominal_turn_ratio_in_pu(TERTIARY_SIDE);
        double angle_tertiary = trans.get_winding_angle_shift_in_deg(TERTIARY_SIDE);
        angle_tertiary = deg2rad(angle_tertiary);

        complex<double> kp(tap_primary*steps_cos(angle_primary),tap_primary*steps_sin(angle_primary)),
                        ks(tap_secondary*steps_cos(angle_secondary),tap_secondary*steps_sin(angle_secondary)),
                        kt(tap_tertiary*steps_cos(angle_tertiary),tap_tertiary*steps_sin(angle_tertiary));

        complex<double> V, I, Vstar;
        complex<double> ypp, yps, ypt, ysp, yss, yst, ytp, yts, ytt, Yeq, Zeq;


        complex<double> kp_store = kp, ks_store = ks, kt_store = kt;
        complex<double> Ym_store = Ym;

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            // voltage at primary side.
            kp = kp/ steps_fast_complex_abs(kp);
            ks = ks/ steps_fast_complex_abs(ks);
            kt = kt/ steps_fast_complex_abs(kt);

            Ym = 0.0;

            // voltage at primary side.
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            I = -Vstar/Zt;
            I /= conj(kt);
            ytp = I;

            // voltage at secondary side
            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp + 1.0/Zt;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            I = -Vstar/Zt;
            I /= conj(kt);
            yts = I;

            // voltage at tertiary side
            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zp + 1.0/Zs;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            ypt = I;

            I = -Vstar/Zs;
            I /= conj(ks);
            yst = I;

            network_BP_matrix.add_entry(p,p,ypp);
            network_BP_matrix.add_entry(p,s,yps);
            network_BP_matrix.add_entry(p,t,ypt);
            network_BP_matrix.add_entry(s,p,ysp);
            network_BP_matrix.add_entry(s,s,yss);
            network_BP_matrix.add_entry(s,t,yst);
            network_BP_matrix.add_entry(t,p,ytp);
            network_BP_matrix.add_entry(t,s,yts);
            network_BP_matrix.add_entry(t,t,ytt);

            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            // voltage at primary side.
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            I = -Vstar/Zt;
            I /= conj(kt);
            ytp = I;

            // voltage at secondary side
            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp + 1.0/Zt;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            I = -Vstar/Zt;
            I /= conj(kt);
            yts = I;

            // voltage at tertiary side
            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zp + 1.0/Zs;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            ypt = I;

            I = -Vstar/Zs;
            I /= conj(ks);
            yst = I;

            if(std::isnan(ypp.real()) or std::isnan(ypp.imag()) or
               std::isnan(yps.real()) or std::isnan(yps.imag()) or
               std::isnan(ypt.real()) or std::isnan(ypt.imag()) or
               std::isnan(ysp.real()) or std::isnan(ysp.imag()) or
               std::isnan(yss.real()) or std::isnan(yss.imag()) or
               std::isnan(yst.real()) or std::isnan(yst.imag()) or
               std::isnan(ytp.real()) or std::isnan(ytp.imag()) or
               std::isnan(yts.real()) or std::isnan(yts.imag()) or
               std::isnan(ytt.real()) or std::isnan(ytt.imag()) )
                cout<<"NAN is detected when adding "<<trans.get_compound_device_name()<<" to BQ"<<endl;
            network_BQ_matrix.add_entry(p,p,ypp);
            network_BQ_matrix.add_entry(p,s,yps);
            network_BQ_matrix.add_entry(p,t,ypt);
            network_BQ_matrix.add_entry(s,p,ysp);
            network_BQ_matrix.add_entry(s,s,yss);
            network_BQ_matrix.add_entry(s,t,yst);
            network_BQ_matrix.add_entry(t,p,ytp);
            network_BQ_matrix.add_entry(t,s,yts);
            network_BQ_matrix.add_entry(t,t,ytt);

            return;
        }


        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            kp = kp/ steps_fast_complex_abs(kp);
            ks = ks/ steps_fast_complex_abs(ks);
            kt = kt/ steps_fast_complex_abs(kt);

            Ym = 0.0;

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            network_BP_matrix.add_entry(p,p,ypp);
            network_BP_matrix.add_entry(p,s,yps);
            network_BP_matrix.add_entry(s,p,ysp);
            network_BP_matrix.add_entry(s,s,yss);


            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            network_BQ_matrix.add_entry(p,p,ypp);
            network_BQ_matrix.add_entry(p,s,yps);
            network_BQ_matrix.add_entry(s,p,ysp);
            network_BQ_matrix.add_entry(s,s,yss);
            return;
        }


        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            kp = kp/ steps_fast_complex_abs(kp);
            ks = ks/ steps_fast_complex_abs(ks);
            kt = kt/ steps_fast_complex_abs(kt);

            Ym = 0.0;

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zt;
            I /= conj(kt);
            ytp = I;

            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            ypt = I;

            network_BP_matrix.add_entry(p,p,ypp);
            network_BP_matrix.add_entry(p,t,ypt);
            network_BP_matrix.add_entry(t,p,ytp);
            network_BP_matrix.add_entry(t,t,ytt);


            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zt;
            I /= conj(kt);
            ytp = I;

            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            ypt = I;

            network_BQ_matrix.add_entry(p,p,ypp);
            network_BQ_matrix.add_entry(p,t,ypt);
            network_BQ_matrix.add_entry(t,p,ytp);
            network_BQ_matrix.add_entry(t,t,ytt);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            kp = kp/ steps_fast_complex_abs(kp);
            ks = ks/ steps_fast_complex_abs(ks);
            kt = kt/ steps_fast_complex_abs(kt);

            Ym = 0.0;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zt;
            I /= conj(kt);
            yts = I;

            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            yst = I;

            network_BP_matrix.add_entry(s,s,yss);
            network_BP_matrix.add_entry(s,t,yst);
            network_BP_matrix.add_entry(t,s,yts);
            network_BP_matrix.add_entry(t,t,ytt);


            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zt;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zt;
            I /= conj(kt);
            yts = I;

            V = 1.0;
            V = V/kt;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zt + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            yst = I;

            network_BQ_matrix.add_entry(s,s,yss);
            network_BQ_matrix.add_entry(s,t,yst);
            network_BQ_matrix.add_entry(t,s,yts);
            network_BQ_matrix.add_entry(t,t,ytt);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            network_BP_matrix.add_entry(p,p,0.0);

            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/kp;
            Zeq = Zp + 1.0/Ym;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            network_BQ_matrix.add_entry(p,p,ypp);


            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            network_BP_matrix.add_entry(s,s,0.0);


            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/ks;
            Zeq = Zs + 1.0/Ym;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            network_BQ_matrix.add_entry(s,s,yss);

            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            network_BP_matrix.add_entry(t,t,0.0);


            kp = steps_fast_complex_abs(kp_store);
            ks = steps_fast_complex_abs(ks_store);
            kt = steps_fast_complex_abs(kt_store);

            Ym = Ym_store;

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());
            Zt = complex<double>(0.0, Zt.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/kt;
            Zeq = Zt + 1.0/Ym;
            I = V/Zeq;
            I /= conj(kt);
            ytt = I;

            network_BQ_matrix.add_entry(t,t,ytt);
            return;
        }
    }
}

void NETWORK_MATRIX::add_two_winding_transformer_to_decoupled_network_v2(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding two-winding transformer '%s' connecting to bus %u and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);
        complex<double> Ym = trans.get_magnetizing_admittance_based_on_winding_norminal_voltage_and_system_base_power_in_pu();

        complex<double> Zp, Zs;
        Zp = 0.5*Zps; Zs = Zp;

        double tap_primary = trans.get_winding_off_nominal_turn_ratio_in_pu(PRIMARY_SIDE);
        double angle_primary = trans.get_winding_angle_shift_in_deg(PRIMARY_SIDE);
        angle_primary = deg2rad(angle_primary);
        double tap_secondary = trans.get_winding_off_nominal_turn_ratio_in_pu(SECONDARY_SIDE);
        double angle_secondary = trans.get_winding_angle_shift_in_deg(SECONDARY_SIDE);
        angle_secondary = deg2rad(angle_secondary);

        complex<double> kp(tap_primary*steps_cos(angle_primary),tap_primary*steps_sin(angle_primary)),
                        ks(tap_secondary*steps_cos(angle_secondary),tap_secondary*steps_sin(angle_secondary));


        complex<double> V, I, Vstar;
        complex<double> ypp, yps, ysp, yss, Yeq, Zeq;

        complex<double> kp_store, ks_store, Ym_store;
        kp_store = kp;
        ks_store = ks;
        Ym_store = Ym;

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            kp = kp/steps_fast_complex_abs(kp);
            ks = ks/steps_fast_complex_abs(ks);
            Ym = 0.0;

            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            network_BP_matrix.add_entry(p,p,ypp);
            network_BP_matrix.add_entry(p,s,yps);
            network_BP_matrix.add_entry(s,p,ysp);
            network_BP_matrix.add_entry(s,s,yss);


            kp = kp_store;
            ks = ks_store;

            kp = steps_fast_complex_abs(kp);
            ks = steps_fast_complex_abs(ks);

            Zp = complex<double>(0.0, Zp.imag());
            Zs = complex<double>(0.0, Zs.imag());

            Ym = 0.0;
            V = 1.0;
            V = V/kp;
            Yeq = Ym + 1.0/Zs;
            Zeq = Zp + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(kp);
            ypp = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zs;
            I /= conj(ks);
            ysp = I;

            V = 1.0;
            V = V/ks;
            Yeq = Ym + 1.0/Zp;
            Zeq = Zs + 1.0/Yeq;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            Vstar = V*(1.0/Yeq)/Zeq;
            I = -Vstar/Zp;
            I /= conj(kp);
            yps = I;

            if(std::isnan(ypp.real()) or std::isnan(ypp.imag()) or
               std::isnan(yps.real()) or std::isnan(yps.imag()) or
               std::isnan(ysp.real()) or std::isnan(ysp.imag()) or
               std::isnan(yss.real()) or std::isnan(yss.imag()) )
                cout<<"NAN is detected when adding "<<trans.get_compound_device_name()<<" to BQ"<<endl;
            network_BQ_matrix.add_entry(p,p,ypp);
            network_BQ_matrix.add_entry(p,s,yps);
            network_BQ_matrix.add_entry(s,p,ysp);
            network_BQ_matrix.add_entry(s,s,yss);

            Ym = Ym_store;
            Ym = complex<double>(0.0, Ym.imag());
            network_BQ_matrix.add_entry(p,p,Ym);

            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==false)
        {
            network_BP_matrix.add_entry(p,p,0.0);

            Ym = complex<double>(0.0, Ym.imag());

            network_BQ_matrix.add_entry(p,p,Ym);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            network_BP_matrix.add_entry(s,s,0.0);

            ks = steps_fast_complex_abs(ks);

            Zps = complex<double>(0.0, Zps.imag());
            Ym = complex<double>(0.0, Ym.imag());

            V = 1.0;
            V = V/ks;
            Zeq = Zps + 1.0/Ym;
            I = V/Zeq;
            I /= conj(ks);
            yss = I;

            network_BQ_matrix.add_entry(s,s,yss);
            return;
        }
    }
}

void NETWORK_MATRIX::add_fixed_shunts_to_decoupled_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<FIXED_SHUNT*> shunts = psdb.get_all_fixed_shunts();

    unsigned int n = shunts.size();

    for(unsigned int i = 0; i!=n; ++i)
        add_fixed_shunt_to_decoupled_network(*(shunts[i]));
}


void NETWORK_MATRIX::add_fixed_shunt_to_decoupled_network(const FIXED_SHUNT& shunt)
{
    if(shunt.get_status()==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding fixed shunt '%s' at bus %u to network Y matrix.", (shunt.get_identifier()).c_str(),
                      shunt.get_shunt_bus());
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int bus = shunt.get_shunt_bus();

        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(bus);

        complex<double> Yshunt = shunt.get_nominal_admittance_shunt_in_pu();

        network_BP_matrix.add_entry(i,i, 0.0);
        network_BQ_matrix.add_entry(i,i, Yshunt);
    }
}



void NETWORK_MATRIX::add_lines_to_dc_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<LINE*> lines = psdb.get_all_lines();

    unsigned int n = lines.size();

    for(unsigned int i = 0; i!=n; ++i)
        add_line_to_dc_network(*(lines[i]));
}

void NETWORK_MATRIX::add_line_to_dc_network(const LINE& line)
{
    if(line.get_sending_side_breaker_status()==true or line.get_receiving_side_breaker_status()==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding line '%s' connecting to bus %u and %u to network Y matrix.", (line.get_identifier()).c_str(),
                      line.get_sending_side_bus(), line.get_receiving_side_bus());
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int sending_bus = line.get_sending_side_bus();
        unsigned int receiving_bus = line.get_receiving_side_bus();

        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(sending_bus);
        unsigned int j = inphno.get_internal_bus_number_of_physical_bus_number(receiving_bus);

        complex<double> Zline = line.get_line_positive_sequence_z_in_pu();
        Zline = complex<double>(0.0, Zline.imag());
        complex<double> Yline = 1.0/Zline;
        if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==true)
        {
            network_DC_B_matrix.add_entry(i,j, -Yline);
            network_DC_B_matrix.add_entry(j,i, -Yline);
            network_DC_B_matrix.add_entry(i,i, Yline);
            network_DC_B_matrix.add_entry(j,j, Yline);
            return;
        }
    }
}


void NETWORK_MATRIX::add_transformers_to_dc_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<TRANSFORMER*> transformers = psdb.get_all_transformers();

    unsigned int n = transformers.size();

    for(unsigned int i = 0; i!=n; ++i)
        add_transformer_to_dc_network(*(transformers[i]));
}

void NETWORK_MATRIX::add_transformer_to_dc_network(const TRANSFORMER& trans)
{
    if(trans.is_two_winding_transformer())
        add_two_winding_transformer_to_dc_network(trans);
    else
        add_three_winding_transformer_to_dc_network(trans);
}

void NETWORK_MATRIX::add_three_winding_transformer_to_dc_network(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or
       trans.get_winding_breaker_status(SECONDARY_SIDE)==true or
       trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding three-winding transformer '%s' connecting to bus %u, %u, and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE), trans.get_winding_bus(TERTIARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);
        unsigned int tertiary_bus = trans.get_winding_bus(TERTIARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);
        unsigned int t = inphno.get_internal_bus_number_of_physical_bus_number(tertiary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);
        complex<double> Zst = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(SECONDARY_SIDE, TERTIARY_SIDE);
        complex<double> Zpt = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, TERTIARY_SIDE);

        complex<double> Zp = 0.5*(Zps+Zpt-Zst),
                         Zs = 0.5*(Zps+Zst-Zpt),
                         Zt = 0.5*(Zpt+Zst-Zps);
        double Xp = Zp.imag();
        double Xs = Zs.imag();
        double Xt = Zt.imag();

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            double Xsigma = Xp*Xs+Xs*Xt+Xt*Xp;
            double Xps = Xsigma/Xt;
            double Xst = Xsigma/Xp;
            double Xtp = Xsigma/Xs;

            complex<double> Yps(0.0, -1.0/Xps);
            complex<double> Yst(0.0, -1.0/Xst);
            complex<double> Ytp(0.0, -1.0/Xtp);

            network_DC_B_matrix.add_entry(p,p,Yps+Ytp);
            network_DC_B_matrix.add_entry(p,s,-Yps);
            network_DC_B_matrix.add_entry(p,t,-Ytp);
            network_DC_B_matrix.add_entry(s,p,-Yps);
            network_DC_B_matrix.add_entry(s,s,Yps+Yst);
            network_DC_B_matrix.add_entry(s,t,-Yst);
            network_DC_B_matrix.add_entry(t,p,-Ytp);
            network_DC_B_matrix.add_entry(t,s,-Yst);
            network_DC_B_matrix.add_entry(t,t,Yst+Ytp);
            return;
        }


        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==false)
        {
            double Xps = Zps.imag();
            complex<double> Yps(0.0, -1.0/Xps);

            network_DC_B_matrix.add_entry(p,p,Yps);
            network_DC_B_matrix.add_entry(p,s,-Yps);
            network_DC_B_matrix.add_entry(s,p,-Yps);
            network_DC_B_matrix.add_entry(s,s,Yps);
            return;
        }


        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==false and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {
            double Xpt = Zpt.imag();
            complex<double> Ypt(0.0, -1.0/Xpt);

            network_DC_B_matrix.add_entry(p,p,Ypt);
            network_DC_B_matrix.add_entry(p,t,-Ypt);
            network_DC_B_matrix.add_entry(t,p,-Ypt);
            network_DC_B_matrix.add_entry(t,t,Ypt);
            return;
        }

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==false and
           trans.get_winding_breaker_status(SECONDARY_SIDE)==true and
           trans.get_winding_breaker_status(TERTIARY_SIDE)==true)
        {

            double Xst = Zst.imag();
            complex<double> Yst(0.0, -1.0/Xst);

            network_DC_B_matrix.add_entry(s,s,Yst);
            network_DC_B_matrix.add_entry(s,t,-Yst);
            network_DC_B_matrix.add_entry(t,s,-Yst);
            network_DC_B_matrix.add_entry(t,t,Yst);
            return;
        }
    }
}

void NETWORK_MATRIX::add_two_winding_transformer_to_dc_network(const TRANSFORMER& trans)
{
    if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true or trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
    {
        /*ostringstream osstream;
        osstream<<"Adding two-winding transformer '%s' connecting to bus %u and %u to network Y matrix.", (trans.get_identifier()).c_str(),
                      trans.get_winding_bus(PRIMARY_SIDE), trans.get_winding_bus(SECONDARY_SIDE));
        toolkit->show_information_with_leading_time_stamp(osstream);
        */

        unsigned int primary_bus = trans.get_winding_bus(PRIMARY_SIDE);
        unsigned int secondary_bus = trans.get_winding_bus(SECONDARY_SIDE);

        unsigned int p = inphno.get_internal_bus_number_of_physical_bus_number(primary_bus);
        unsigned int s = inphno.get_internal_bus_number_of_physical_bus_number(secondary_bus);

        complex<double> Zps = trans.get_leakage_impedance_between_windings_based_on_system_base_power_in_pu(PRIMARY_SIDE, SECONDARY_SIDE);

        double Xps = Zps.imag();
        complex<double> Yps(0.0, -1.0/Xps);

        if(trans.get_winding_breaker_status(PRIMARY_SIDE)==true and trans.get_winding_breaker_status(SECONDARY_SIDE)==true)
        {
            network_DC_B_matrix.add_entry(p,p,Yps);
            network_DC_B_matrix.add_entry(p,s,-Yps);
            network_DC_B_matrix.add_entry(s,p,-Yps);
            network_DC_B_matrix.add_entry(s,s,Yps);
        }
    }
}


void NETWORK_MATRIX::add_bus_fault_to_dynamic_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<BUS*> buses = psdb.get_all_buses();

    unsigned int n = buses.size();

    BUS* bus;

    for(unsigned int i = 0; i!=n; ++i)
    {
        bus = buses[i];
        if(bus->get_bus_type()!=OUT_OF_SERVICE)
        {
            if(not bus->is_faulted())
                continue;
            else
            {
                FAULT fault = bus->get_fault();
                complex<double> y = fault.get_fault_shunt_in_pu();
                unsigned int j = inphno.get_internal_bus_number_of_physical_bus_number(bus->get_bus_number());
                this_Y_matrix_pointer->add_entry(j,j, y);
            }
        }
    }
}


void NETWORK_MATRIX::add_lines_to_dynamic_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<LINE*> lines = psdb.get_all_lines();

    unsigned int n = lines.size();

    for(unsigned int i = 0; i!=n; ++i)
    {
        if(not lines[i]->is_faulted())
           add_line_to_network(*(lines[i]));
        else
           add_faulted_line_to_dynamic_network(*(lines[i]));
    }
}

void NETWORK_MATRIX::add_faulted_line_to_dynamic_network(const LINE& line)
{
    if(line.get_sending_side_breaker_status()==true or line.get_receiving_side_breaker_status()==true)
    {
        if(line.is_faulted())
        {
            /*ostringstream osstream;
            osstream<<"Adding line '%s' connecting to bus %u and %u to network Y matrix.", (line.get_identifier()).c_str(),
                          line.get_sending_side_bus(), line.get_receiving_side_bus());
            toolkit->show_information_with_leading_time_stamp(osstream);*/

            unsigned int sending_bus = line.get_sending_side_bus();
            unsigned int receiving_bus = line.get_receiving_side_bus();

            unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(sending_bus);
            unsigned int j = inphno.get_internal_bus_number_of_physical_bus_number(receiving_bus);

            complex<double> Zline = line.get_line_positive_sequence_z_in_pu();
            complex<double> Yline = line.get_line_positive_sequence_y_in_pu();
            complex<double> Yshunt_sending = line.get_shunt_positive_sequence_y_at_sending_side_in_pu();
            complex<double> Yshunt_receiving = line.get_shunt_positive_sequence_y_at_receiving_side_in_pu();

            unsigned int nfault = line.get_fault_count();
            // take care of fault at two ends
            FAULT fault = line.get_fault_at_location(line.get_sending_side_bus(), 0.0);
            Yshunt_sending += fault.get_fault_shunt_in_pu();

            fault = line.get_fault_at_location(line.get_sending_side_bus(), 1.0);
            Yshunt_receiving += fault.get_fault_shunt_in_pu();

            vector<SUBLINE> sublines;

            double starting_fault_location;
            unsigned int n;

            for(unsigned int k=0; k!=nfault; ++k)
            {
                double this_fault_location = line.get_fault_location_of_fault(k);
                if(this_fault_location != 0.0 and this_fault_location != 1.0)
                {
                    n = sublines.size();
                    if(n==0)
                        starting_fault_location = 0.0;
                    else
                        starting_fault_location = sublines[n-1].to_loc;

                    SUBLINE sl;
                    sl.from_loc = starting_fault_location;
                    sl.to_loc = this_fault_location;
                    sublines.push_back(sl);
                }
            }
            n = sublines.size();
            if(n==0)
                starting_fault_location = 0.0;
            else
                starting_fault_location = sublines[n-1].to_loc;

            SUBLINE sl;
            sl.from_loc = starting_fault_location;
            sl.to_loc = 1.0;
            sublines.push_back(sl);

            complex<double> V, Y, Z, y, z, I;
            if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==true)
            {
                V = 1.0; // source at sending side, receiving side shorted
                Y = 0.0; Z = 0;
                n = sublines.size();
                double line_length = sublines[n-1].to_loc - sublines[n-1].from_loc;
                y = Yline*line_length; z = Zline*line_length;
                Z = z; Y = 1.0/Z+y*0.5;
                for(int k=n-2; k>=0; --k)
                {
                    double fault_location = sublines[k].to_loc;
                    line_length = sublines[k].to_loc - sublines[k].from_loc;
                    fault = line.get_fault_at_location(line.get_sending_side_bus(), fault_location);
                    complex<double> yfault = fault.get_fault_shunt_in_pu();
                    y = Yline*line_length; z = Zline*line_length;
                    Y = Y+y*0.5+yfault; Z = z+1.0/Y;
                    Y = 1.0/Z+y*0.5;
                }
                Y = Y+Yshunt_sending;
                I = V*Y;
                this_Y_matrix_pointer->add_entry(i,i, I);
                Y = Yshunt_sending;
                I = I-V*Y;
                for(unsigned int k=0; k!=n; ++k)
                {
                    double fault_location = sublines[k].to_loc;
                    line_length = sublines[k].to_loc - sublines[k].from_loc;
                    y = Yline*line_length; z = Zline*line_length;
                    I = I-V*y*0.5;
                    V = V-I*z;
                    if(fault_location!=1.0)
                    {
                        I = I - V*y*0.5;
                        fault = line.get_fault_at_location(line.get_sending_side_bus(), fault_location);
                        complex<double> yfault = fault.get_fault_shunt_in_pu();
                        I = I - V*yfault;
                    }
                }
                this_Y_matrix_pointer->add_entry(j,i, -I);
                this_Y_matrix_pointer->add_entry(i,j, -I);

                V = 1.0; // source at receiving side, sending side shorted

                n = sublines.size();
                line_length = sublines[0].to_loc - sublines[0].from_loc;
                y = Yline*line_length; z = Zline*line_length;
                Z = z; Y = 1.0/Z+y*0.5;
                for(unsigned int k=1; k!=n; ++k)
                {
                    double fault_location = sublines[k].from_loc;
                    line_length = sublines[k].to_loc - sublines[k].from_loc;
                    y = Yline*line_length; z = Zline*line_length;
                    fault = line.get_fault_at_location(line.get_sending_side_bus(), fault_location);
                    complex<double> yfault = fault.get_fault_shunt_in_pu();
                    Y = Y+y*0.5+yfault; Z = z+1.0/Y;
                    Y = 1.0/Z+y*0.5;
                }
                Y = Y+Yshunt_receiving;
                I = V*Y;
                this_Y_matrix_pointer->add_entry(j,j, I);
                return;
            }

            if(line.get_sending_side_breaker_status()==true and line.get_receiving_side_breaker_status()==false)
            {
                V = 1.0; // source at sending side
                Y = Yshunt_receiving; Z = 0;
                n = sublines.size();
                for(int k=n-1; k>=0; --k)
                {
                    double fault_location = sublines[k].from_loc;
                    double line_length = sublines[k].to_loc - sublines[k].from_loc;
                    complex<double> yfault = 0.0;
                    if(fault_location!=0.0)
                    {
                        fault = line.get_fault_at_location(line.get_sending_side_bus(), fault_location);
                        yfault = fault.get_fault_shunt_in_pu();
                    }
                    y = Yline*line_length; z = Zline*line_length;
                    Y = Y+y*0.5; Z = z+1.0/Y;
                    Y = 1.0/Z+y*0.5+yfault;
                }
                Y = Y+Yshunt_sending;
                I = V*Y;
                this_Y_matrix_pointer->add_entry(i,i, I);
                return;
            }

            if(line.get_sending_side_breaker_status()==false and line.get_receiving_side_breaker_status()==true)
            {
                V = 1.0; // source at receiving side
                Y = Yshunt_sending; Z = 0;
                n = sublines.size();
                int nint = int(n);
                for(int k=0; k!=nint; ++k)
                {
                    double fault_location = sublines[k].to_loc;
                    double line_length = sublines[k].to_loc - sublines[k].from_loc;
                    complex<double> yfault = 0.0;
                    if(fault_location!=1.0)
                    {
                        fault = line.get_fault_at_location(line.get_sending_side_bus(), fault_location);
                        yfault = fault.get_fault_shunt_in_pu();
                    }
                    y = Yline*line_length; z = Zline*line_length;
                    Y = Y+y*0.5; Z = z+1.0/Y;
                    Y = 1.0/Z+y*0.5+yfault;
                }
                Y = Y+Yshunt_receiving;
                I = V*Y;
                this_Y_matrix_pointer->add_entry(j,j, I);
                return;
            }
        }
        else // line is not faulted
            add_line_to_network(line);
    }
}

void NETWORK_MATRIX::add_generators_to_dynamic_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<GENERATOR*> generators = psdb.get_all_generators();

    unsigned int n= generators.size();

    for(unsigned int i=0; i!=n; ++i)
        add_generator_to_dynamic_network(*(generators[i]));
}

void NETWORK_MATRIX::add_generator_to_dynamic_network(const GENERATOR& gen)
{
    if(gen.get_status()==true)
    {
        POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
        complex<double> Z = gen.get_source_impedance_in_pu();
        double one_over_mbase = gen.get_one_over_mbase_in_one_over_MVA();
        double sbase = psdb.get_system_base_power_in_MVA();
        Z *= (one_over_mbase*sbase);

        unsigned int bus = gen.get_generator_bus();
        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(bus);
        this_Y_matrix_pointer->add_entry(i,i,1.0/Z);
    }
}

void NETWORK_MATRIX::add_wt_generators_to_dynamic_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<WT_GENERATOR*> generators = psdb.get_all_wt_generators();

    unsigned int n= generators.size();

    for(unsigned int i=0; i!=n; ++i)
        add_wt_generator_to_dynamic_network(*(generators[i]));
}

void NETWORK_MATRIX::add_wt_generator_to_dynamic_network(WT_GENERATOR& gen)
{
    if(gen.get_status()==true)
    {
        WT_GENERATOR_MODEL* genmodel = gen.get_wt_generator_model();
        if(genmodel!=NULL)
        {
            if(genmodel->is_voltage_source())
            {
                POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
                complex<double> Z = gen.get_source_impedance_in_pu();
                double one_over_mbase = gen.get_one_over_mbase_in_one_over_MVA();
                double sbase = psdb.get_system_base_power_in_MVA();
                Z *= (one_over_mbase*sbase);

                unsigned int bus = gen.get_generator_bus();
                unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(bus);
                this_Y_matrix_pointer->add_entry(i,i,1.0/Z);
            }
            //else // is current source
            //    return;
        }
        else
        {
            ostringstream osstream;
            osstream<<"Error. No WT_GENERATOR_MODEL is provided for "<<gen.get_compound_device_name()<<endl
                    <<"Its source impedance will not be added to network matrix.";
            toolkit->show_information_with_leading_time_stamp(osstream);
            return;
        }
    }
}


void NETWORK_MATRIX::add_motor_loads_to_dynamic_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<LOAD*> loads = psdb.get_all_loads();

    unsigned int n= loads.size();

    for(unsigned int i=0; i!=n; ++i)
    {
        LOAD* load = loads[i];
        LOAD_MODEL* model = load->get_load_model();
        if(model!=NULL)
        {
            if(model->is_voltage_source())
                add_motor_load_to_dynamic_network(*load);
        }
    }
}

void NETWORK_MATRIX::add_motor_load_to_dynamic_network(const LOAD& load)
{
    LOAD_MODEL* model = load.get_load_model();
    if(model!=NULL)
    {
        if(model->is_voltage_source())
        {
            complex<double> ysource = model->get_dynamic_source_admittance_in_pu_based_on_SBASE();
            complex<double> yshunt = model->get_additional_admittance_in_pu_based_on_SBASE();
            complex<double> y = ysource+yshunt;
            if(y!=0.0)
            {
                unsigned int bus = load.get_load_bus();
                unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(bus);
                this_Y_matrix_pointer->add_entry(i,i,y);
            }
        }
    }
}


void NETWORK_MATRIX::add_vsc_hvdcs_to_dynamic_network()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    vector<VSC_HVDC*> vsc_hvdcs = psdb.get_all_vsc_hvdcs();

    unsigned int n= vsc_hvdcs.size();

    for(unsigned int i=0; i!=n; ++i)
    {
        VSC_HVDC* vsc_hvdc = vsc_hvdcs[i];
        add_vsc_hvdc_to_dynamic_network(*vsc_hvdc);
    }
}

void NETWORK_MATRIX::add_vsc_hvdc_to_dynamic_network(const VSC_HVDC& vsc_hvdc)
{
    if(vsc_hvdc.get_status()==true)
    {
        unsigned int n_converter = vsc_hvdc.get_converter_count();
        for(unsigned int i=0; i!=n_converter; ++i)
        {
            if(vsc_hvdc.get_converter_status(i)==true)
            {
                VSC_HVDC_CONVERTER_MODEL* model = vsc_hvdc.get_vsc_hvdc_converter_model(i);
                if(model!=NULL)
                {
                    if(model->is_voltage_source())
                    {
                        complex<double> y = vsc_hvdc.get_converter_Norton_admittance_as_voltage_source(i);
                        unsigned int bus = vsc_hvdc.get_converter_ac_bus(i);
                        unsigned int i = inphno.get_internal_bus_number_of_physical_bus_number(bus);
                        this_Y_matrix_pointer->add_entry(i,i,y);
                    }
                }
            }
        }
    }
}

void NETWORK_MATRIX::optimize_network_ordering()
{
    initialize_physical_internal_bus_pair();
    build_network_Y_matrix();
    if(toolkit->is_optimize_network_enabled())
    {
        reorder_physical_internal_bus_pair();
        /*for(unsigned int i=0; i<1; ++i)
        {
            reorder_physical_internal_bus_pair();
            build_network_Y1_matrix();
        }*/
    }
}

bool NETWORK_MATRIX::is_condition_ok() const
{
    return true;
}

void NETWORK_MATRIX::initialize_physical_internal_bus_pair()
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();
    psdb.check_device_status_for_out_of_service_buses();
    vector<BUS*> buses = psdb.get_all_buses();

    unsigned int nbus = buses.size();

    if(nbus!=0)
    {
        inphno.clear();

        unsigned int bus_number;
        unsigned int internal_bus_number = 0;
        for(unsigned int i=0; i!=nbus; ++i)
        {
            if(buses[i]->get_bus_type()!=OUT_OF_SERVICE)
            {
                bus_number = buses[i]->get_bus_number();
                inphno.set_physical_internal_bus_number_pair(bus_number, internal_bus_number);
                internal_bus_number ++;
            }
        }
    }
}

void NETWORK_MATRIX::reorder_physical_internal_bus_pair()
{
    vector<unsigned int> permutation = network_Y1_matrix.get_reorder_permutation();
    inphno.update_with_new_internal_bus_permutation(permutation);
    ostringstream osstream;
    osstream<<"Network internal bus numbers are optimized.";
    toolkit->show_information_with_leading_time_stamp(osstream);

    /*os<<"After optimizing the network with network optimizer, the internal buses are listed as follows");
    toolkit->show_information_with_leading_time_stamp(osstream);

    osstream<<"internal   physical   storage");
    toolkit->show_information_with_leading_time_stamp(osstream);

    unsigned int bus, index, nbus;

    nbus = get_in_service_bus_count();

    for(unsigned int i=0; i!=nbus; ++i)
    {
        bus = get_physical_bus_number_of_internal_bus(i);
        index = get_bus_index(bus);
        osstream<<"%-10u %-10u %-10u",i, bus, index);
        toolkit->show_information_with_leading_time_stamp(osstream);
    }*/

    /*os<<"Permutation for reorder internal bus:");
    toolkit->show_information_with_leading_time_stamp(osstream);

    unsigned int nperm = permutation.size();
    for(unsigned int i=0; i!=nperm; ++i)
    {
        osstream<<"%u ---> %u",i,permutation[i]);
        toolkit->show_information_with_leading_time_stamp(osstream);
    }
    */

}

void NETWORK_MATRIX::check_network_connectivity(bool remove_void_island)
{
    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();

    vector< vector<unsigned int> > islands = get_islands_with_physical_bus_number();

    unsigned int nislands = islands.size();

    ostringstream osstream;
    osstream<<"There are "<<nislands<<" islands.";
    toolkit->show_information_with_leading_time_stamp(osstream);

    unsigned int physical_bus;
    bool there_is_slack_bus_in_island;
    BUS* bus;
    for(unsigned int i=0; i!=nislands; ++i)
    {
        osstream<<"Island "<<i<<":";
        toolkit->show_information_with_leading_time_stamp(osstream);

        osstream<<"Areas in island "<<i<<":\n";
        unsigned int nbus = islands[i].size();
        set<unsigned int> areas_in_island;
        for(unsigned int j=0; j!=nbus; ++j)
        {
            physical_bus = islands[i][j];
            bus = psdb.get_bus(physical_bus);
            unsigned int area = bus->get_area_number();
            areas_in_island.insert(area);
        }
        set<unsigned int>::iterator iter;
        unsigned int area_count = 0;
        for(iter = areas_in_island.begin() ; iter != areas_in_island.end() ; ++iter)
        {
            area_count++;
            unsigned int area = *iter;
            osstream<<"("<<setw(3)<<area_count<<") "<<area<<" ["<<psdb.area_number2area_name(area)<<"]\n";
        }
        toolkit->show_information_with_leading_time_stamp(osstream);


        osstream<<"Physical buses in island "<<i<<":";
        toolkit->show_information_with_leading_time_stamp(osstream);
        there_is_slack_bus_in_island = false;
        for(unsigned int j=0; j!=nbus; ++j)
        {
            physical_bus = islands[i][j];
            bus = psdb.get_bus(physical_bus);
            unsigned int area = bus->get_area_number();
            osstream<<physical_bus<<" ["<<bus->get_bus_name()<<"  "<<bus->get_base_voltage_in_kV()<<"kV] AREA "<<area<<"["<<psdb.area_number2area_name(area)<<"]";
            if(bus->get_bus_type()==SLACK_TYPE)
            {
                osstream<<" (Slack bus)";
                there_is_slack_bus_in_island = true;
            }
            toolkit->show_information_with_leading_time_stamp(osstream);
        }
        if(not there_is_slack_bus_in_island)
        {
            osstream<<"No slack bus is found in island "<<i<<".";
            toolkit->show_information_with_leading_time_stamp(osstream);
            if(remove_void_island==true)
            {
                for(unsigned int j=0; j!=nbus; ++j)
                {
                    physical_bus = islands[i][j];
                    bus = psdb.get_bus(physical_bus);
                    bus->set_bus_type(OUT_OF_SERVICE);
                }
                osstream<<"All buses in island "<<i<<" are set as OUT OF SERVICE.";
                toolkit->show_information_with_leading_time_stamp(osstream);
            }
        }
    }
}

vector< vector<unsigned int> > NETWORK_MATRIX::get_islands_with_internal_bus_number()
{
    vector< vector<unsigned int> > islands;

    if(inphno.empty())
        initialize_physical_internal_bus_pair();

    POWER_SYSTEM_DATABASE& psdb = toolkit->get_power_system_database();

    //build_network_Y1_matrix();
    //network_Y1_matrix.report_brief();

    unsigned int nbus = psdb.get_in_service_bus_count();

    vector<bool> bus_searched_flag;
    bus_searched_flag.reserve(nbus);

    for(unsigned int i=0; i!=nbus; ++i)
    {
        unsigned int physical_bus = get_physical_bus_number_of_internal_bus(i);
        if(physical_bus!=INDEX_NOT_EXIST)
            bus_searched_flag.push_back(false);
        else
            bus_searched_flag.push_back(true);
    }

    while(true)
    {
        bool new_island_found = false;
        unsigned int first_bus_in_new_island;

        for(unsigned int i=0; i!=nbus; ++i)
        {
            if(bus_searched_flag[i]==false)
            {
                new_island_found = true;
                first_bus_in_new_island = i;
                break;
            }
        }
        if(new_island_found==true)
        {
            vector<unsigned int> this_island;
            this_island.reserve(nbus);

            this_island.push_back(first_bus_in_new_island);
            bus_searched_flag[first_bus_in_new_island] = true;

            int searching_bus=0;
            unsigned int nbus_in_island = this_island.size();
            for(unsigned int i=0; i!=nbus_in_island; ++i)
            {
                searching_bus = this_island[i];
                int k_start = network_Y1_matrix.get_starting_index_of_column(searching_bus);
                int k_end = network_Y1_matrix.get_starting_index_of_column(searching_bus+1);
                int row_bus;
                for(int k=k_start; k!=k_end; ++k)
                {
                    row_bus = network_Y1_matrix.get_row_number_of_entry_index(k);
                    if(row_bus!=searching_bus)
                    {
                        if(bus_searched_flag[row_bus]==false)
                        {
                            this_island.push_back(row_bus);
                            nbus_in_island = this_island.size();
                            bus_searched_flag[row_bus] = true;
                        }
                    }
                }
            }
            islands.push_back(this_island);
        }
        else
            break;
    }
    return islands;
}


vector< vector<unsigned int> > NETWORK_MATRIX::get_islands_with_physical_bus_number()
{
    vector< vector<unsigned int> > islands = get_islands_with_internal_bus_number();

    unsigned int nislands = islands.size();
    for(unsigned int i=0; i!= nislands; ++i)
    {
        unsigned int nbus_in_island = islands[i].size();
        for(unsigned int j=0; j!=nbus_in_island; ++j)
        {
            islands[i][j] = get_physical_bus_number_of_internal_bus(islands[i][j]);
        }
    }

    return islands;
}

unsigned int NETWORK_MATRIX::get_internal_bus_number_of_physical_bus(unsigned int bus) const
{
    return inphno.get_internal_bus_number_of_physical_bus_number(bus);
}

unsigned int NETWORK_MATRIX::get_physical_bus_number_of_internal_bus(unsigned int bus) const
{
    return inphno.get_physical_bus_number_of_internal_bus_number(bus);
}

void NETWORK_MATRIX::report_network_matrix()
{
    ostringstream osstream;

    osstream<<"Network Y matrix lists begin:";
    toolkit->show_information_with_leading_time_stamp(osstream);

    set_this_Y_and_Z_matrix_as(network_Y1_matrix);
    report_network_matrix_common();
}

void NETWORK_MATRIX::report_decoupled_network_matrix() const
{
    ostringstream osstream;

    osstream<<"Network decoupled B matrix lists begin:";
    toolkit->show_information_with_leading_time_stamp(osstream);

    osstream<<"row   [  bus  ]  column[  bus  ]     BP       BQ";
    toolkit->show_information_with_leading_time_stamp(osstream);

    unsigned int i, ibus, jbus;
    unsigned int n = network_BP_matrix.get_matrix_size();
    int k_starting, k_ending;
    k_starting = 0;
    complex<double> y;
    double bp, bq;
    for(unsigned int j=0; j!=n; ++j)
    {
        k_ending = network_BP_matrix.get_starting_index_of_column(j+1);
        for(int k=k_starting; k!=k_ending; ++k)
        {
            y = network_BP_matrix.get_entry_value(k);
            bp = y.imag();
            y = network_BQ_matrix.get_entry_value(k);
            bq = y.imag();
            i = network_BP_matrix.get_row_number_of_entry_index(k);
            ibus = get_physical_bus_number_of_internal_bus(i);
            jbus = get_physical_bus_number_of_internal_bus(j);

            osstream<<setw(6)<<i<<"["
              <<setw(7)<<ibus<<"]  "
              <<setw(6)<<j<<"["
              <<setw(7)<<jbus<<"]  "
              <<setw(10)<<setprecision(6)<<fixed<<bp<<", "
              <<setw(10)<<setprecision(6)<<fixed<<bq<<endl;

            toolkit->show_information_with_leading_time_stamp(osstream);
        }
        k_starting = k_ending;
    }
    osstream<<"Network decoupled B matrix lists finished.";
    toolkit->show_information_with_leading_time_stamp(osstream);
}

void NETWORK_MATRIX::report_dc_network_matrix() const
{
    ostringstream osstream;

    osstream<<"DC network B matrix lists begin:";
    toolkit->show_information_with_leading_time_stamp(osstream);

    osstream<<"row   [  bus  ]  column[  bus  ]     B";
    toolkit->show_information_with_leading_time_stamp(osstream);

    unsigned int i, ibus, jbus;
    unsigned int n = network_DC_B_matrix.get_matrix_size();
    int k_starting, k_ending;
    k_starting = 0;
    complex<double> y;
    double b;
    for(unsigned int j=0; j!=n; ++j)
    {
        k_ending = network_DC_B_matrix.get_starting_index_of_column(j+1);
        for(int k=k_starting; k!=k_ending; ++k)
        {
            y = network_DC_B_matrix.get_entry_value(k);
            b = y.imag();
            i = network_DC_B_matrix.get_row_number_of_entry_index(k);
            ibus = get_physical_bus_number_of_internal_bus(i);
            jbus = get_physical_bus_number_of_internal_bus(j);

            osstream<<setw(6)<<i<<"["
              <<setw(7)<<ibus<<"]  "
              <<setw(6)<<j<<"["
              <<setw(7)<<jbus<<"]  "
              <<setw(10)<<setprecision(6)<<fixed<<b<<endl;

            toolkit->show_information_with_leading_time_stamp(osstream);
        }
        k_starting = k_ending;
    }
    osstream<<"DC network B matrix lists finished.";
    toolkit->show_information_with_leading_time_stamp(osstream);
}

void NETWORK_MATRIX::report_dynamic_network_matrix()
{
    ostringstream osstream;

    osstream<<"Network dynamic Y matrix lists begin:";
    toolkit->show_information_with_leading_time_stamp(osstream);

    set_this_Y_and_Z_matrix_as(network_Y1_matrix);
	report_network_matrix_common();
}

void NETWORK_MATRIX::report_network_matrix_common() const
{
    ostringstream osstream;

    osstream<<"row   [  bus  ]  column[  bus  ]     real       imaginary";
    toolkit->show_information_with_leading_time_stamp(osstream);

    unsigned int i, ibus, jbus;
    unsigned int n = this_Y_matrix_pointer->get_matrix_size();
    int k_starting, k_ending;
    k_starting = 0;
    complex<double> y;
    for(unsigned int j=0; j!=n; ++j)
    {
        k_ending = this_Y_matrix_pointer->get_starting_index_of_column(j+1);
        for(int k=k_starting; k!=k_ending; ++k)
        {
            y = this_Y_matrix_pointer->get_entry_value(k);
            i = this_Y_matrix_pointer->get_row_number_of_entry_index(k);
            ibus = get_physical_bus_number_of_internal_bus(i);
            jbus = get_physical_bus_number_of_internal_bus(j);

            osstream<<setw(6)<<i<<"["
              <<setw(7)<<ibus<<"]  "
              <<setw(6)<<j<<"["
              <<setw(7)<<jbus<<"]  "
              <<setw(10)<<setprecision(6)<<fixed<<y.real()<<", "
              <<setw(10)<<setprecision(6)<<fixed<<y.imag()<<endl;
            toolkit->show_information_with_leading_time_stamp(osstream);
        }
        k_starting = k_ending;
    }
    osstream<<"Network matrix lists finished.";
    toolkit->show_information_with_leading_time_stamp(osstream);
}


void NETWORK_MATRIX::save_network_Y_matrix_to_file(const string& filename, bool export_full_matrix)
{
    ostringstream osstream;
    ofstream file(filename);
    if(file.is_open())
    {
        set_this_Y_and_Z_matrix_as(network_Y1_matrix);
        save_network_matrix_common(file);
        file.close();

        ofstream full_file("full_"+filename);
        if(full_file.is_open())
        {
            save_full_network_matrix_common(full_file);
            full_file.close();
        }
    }
    else
    {
        osstream<<"File '"<<filename<<"' cannot be opened for saving network matrix to file."<<endl
          <<"No network matrix will be exported.";
        toolkit->show_information_with_leading_time_stamp(osstream);
    }
}


void NETWORK_MATRIX::save_decoupled_network_B_matrix_to_file(const string& filename) const
{
    ostringstream osstream;

    ofstream file(filename);
    if(file.is_open())
    {
        file<<"ROW,ROW_BUS,COLUMN,COLUMN_BUS,BP,BQ"<<endl;

        unsigned int i, ibus, jbus;
        unsigned int n = network_BP_matrix.get_matrix_size();
        int k_starting, k_ending;
        k_starting = 0;
        complex<double> yp, yq;
        char buffer[1000];
        for(unsigned int j=0; j!=n; ++j)
        {
            k_ending = network_BP_matrix.get_starting_index_of_column(j+1);
            for(int k=k_starting; k!=k_ending; ++k)
            {
                yp = network_BP_matrix.get_entry_value(k);
                yq = network_BQ_matrix.get_entry_value(k);
                i = network_BP_matrix.get_row_number_of_entry_index(k);
                ibus = get_physical_bus_number_of_internal_bus(i);
                jbus = get_physical_bus_number_of_internal_bus(j);

                snprintf(buffer, 1000, "%u,%u,%u,%u,%.14f,%.14f",i, ibus, j, jbus, yp.imag(), yq.imag());
                file<<buffer<<endl;
                /*file<<i<<","<<ibus<<","<<j<<","<<jbus<<","
                    <<setprecision(14)<<fixed<<yp.imag()<<","
                    <<setprecision(14)<<fixed<<yq.imag()<<endl;*/
            }
            k_starting = k_ending;
        }
        file.close();
    }
    else
    {
        osstream<<"File '"<<filename<<"' cannot be opened for saving decoupled network matrix to file."<<endl
          <<"No decoupled network matrix will be exported.";
        toolkit->show_information_with_leading_time_stamp(osstream);
    }
}

void NETWORK_MATRIX::save_dc_network_B_matrix_to_file(const string& filename) const
{
    ostringstream osstream;

    ofstream file(filename);
    if(file.is_open())
    {
        file<<"ROW,ROW_BUS,COLUMN,COLUMN_BUS,B"<<endl;

        unsigned int i, ibus, jbus;
        unsigned int n = network_DC_B_matrix.get_matrix_size();
        int k_starting, k_ending;
        k_starting = 0;
        complex<double> y;
        char buffer[1000];
        for(unsigned int j=0; j!=n; ++j)
        {
            k_ending = network_DC_B_matrix.get_starting_index_of_column(j+1);
            for(int k=k_starting; k!=k_ending; ++k)
            {
                y = network_DC_B_matrix.get_entry_value(k);
                i = network_DC_B_matrix.get_row_number_of_entry_index(k);
                ibus = get_physical_bus_number_of_internal_bus(i);
                jbus = get_physical_bus_number_of_internal_bus(j);

                snprintf(buffer, 1000, "%u,%u,%u,%u,%.14f",i, ibus, j, jbus, y.imag());
                file<<buffer<<endl;
                /*file<<i<<","<<ibus<<","<<j<<","<<jbus<<","
                    <<setprecision(14)<<fixed<<y.imag()<<endl;*/
            }
            k_starting = k_ending;
        }
        file.close();
    }
    else
    {
        osstream<<"File '"<<filename<<"' cannot be opened for saving DC network matrix to file."<<endl
          <<"No DC network matrix will be exported.";
        toolkit->show_information_with_leading_time_stamp(osstream);
    }
}

void NETWORK_MATRIX::save_dynamic_network_Y_matrix_to_file(const string& filename)
{
    ostringstream osstream;

    ofstream file(filename);
    if(file.is_open())
    {
        set_this_Y_and_Z_matrix_as(network_Y1_matrix);
        save_network_matrix_common(file);
        file.close();
    }
    else
    {
        osstream<<"File '"<<filename<<"' cannot be opened for saving dynamic network matrix to file."<<endl
          <<"No dynamic network matrix will be exported.";
        toolkit->show_information_with_leading_time_stamp(osstream);
    }
}


void NETWORK_MATRIX::save_network_matrix_common(ofstream& file) const
{
    file<<"ROW,ROW_BUS,COLUMN,COLUMN_BUS,REAL,IMAGINARY"<<endl;

    unsigned int i, ibus, jbus;
    unsigned int n = this_Y_matrix_pointer->get_matrix_size();
    int k_starting, k_ending;
    k_starting = 0;
    complex<double> y;
    char buffer[1000];
    for(unsigned int j=0; j!=n; ++j)
    {
        k_ending = this_Y_matrix_pointer->get_starting_index_of_column(j+1);
        for(int k=k_starting; k!=k_ending; ++k)
        {
            y = this_Y_matrix_pointer->get_entry_value(k);
            i = this_Y_matrix_pointer->get_row_number_of_entry_index(k);
            ibus = get_physical_bus_number_of_internal_bus(i);
            jbus = get_physical_bus_number_of_internal_bus(j);

            snprintf(buffer, 1000, "%u,%u,%u,%u,%.14f,%.14f",i, ibus, j, jbus, y.real(), y.imag());
            file<<buffer<<endl;
            /*file<<i<<","<<ibus<<","<<j<<","<<jbus<<","
                <<setprecision(14)<<fixed<<y.real()<<","
                <<setprecision(14)<<fixed<<y.imag()<<endl;*/
        }
        k_starting = k_ending;
    }
}


void NETWORK_MATRIX::save_full_network_matrix_common(ofstream& file) const
{
    unsigned int i, ibus;
    unsigned int n = this_Y_matrix_pointer->get_matrix_size();
    complex<double> y;

    file<<"internal,";
    for(i=0; i<n; ++i)
        file<<","<<num2str(i);
    file<<endl;
    file<<",physical";
    for(i=0; i<n; ++i)
        file<<","<<num2str(get_physical_bus_number_of_internal_bus(i));
    file<<endl;
    for(i=0; i<n; ++i)
    {
        ibus = get_physical_bus_number_of_internal_bus(i);
        file<<num2str(i)<<","<<num2str(ibus);
        for(unsigned int j=0; j<n; ++j)
        {
            y = this_Y_matrix_pointer->get_entry_value(i,j);
            if(y==0.0)
            {
                file<<",";
            }
            else
            {
                if(y.imag()>=0.0)
                {
                    if(y.real()!=0.0)
                        file<<","<<num2str(y.real())<<"+"<<num2str(y.imag())<<"i";
                    else
                        file<<","<<num2str(y.imag())<<"i";
                }
                else
                {
                    if(y.real()!=0.0)
                        file<<","<<num2str(y.real())<<"-"<<num2str(-y.imag())<<"i";
                    else
                        file<<",-"<<num2str(-y.imag())<<"i";
                }
            }
        }
        file<<endl;
    }
}


void NETWORK_MATRIX::save_network_Z_matrix_to_file(const string& filename) const
{
    ostringstream osstream;

    ofstream file(filename);
    if(file.is_open())
    {
        file<<"ROW,ROW_BUS,COLUMN,COLUMN_BUS,REAL,IMAGINARY"<<endl;

        unsigned int i, ibus, jbus;
        unsigned int n = network_Z1_matrix.get_matrix_size();
        int k_starting, k_ending;
        k_starting = 0;
        complex<double> z;
        char buffer[1000];
        for(unsigned int j=0; j!=n; ++j)
        {
            k_ending = network_Z1_matrix.get_starting_index_of_column(j+1);
            for(int k=k_starting; k!=k_ending; ++k)
            {
                z = network_Z1_matrix.get_entry_value(k);
                i = network_Z1_matrix.get_row_number_of_entry_index(k);
                ibus = get_physical_bus_number_of_internal_bus(i);
                jbus = get_physical_bus_number_of_internal_bus(j);

                if(z.real()!=INFINITE_THRESHOLD and z.imag()!=INFINITE_THRESHOLD)
                {
                    snprintf(buffer, 1000, "%u,%u,%u,%u,%.14f,%.14f",i, ibus, j, jbus, z.real(), z.imag());
                    file<<buffer<<endl;
                }
            }
            k_starting = k_ending;
        }
        file.close();
    }
    else
    {
        osstream<<"File '"<<filename<<"' cannot be opened for saving network Z matrix to file."<<endl
          <<"No network Z matrix will be exported.";
        toolkit->show_information_with_leading_time_stamp(osstream);
    }
}

void NETWORK_MATRIX::report_physical_internal_bus_number_pair() const
{
    inphno.report();
}

unsigned int NETWORK_MATRIX::get_memory_usage_in_bytes()
{
    return network_Y1_matrix.get_memory_usage_in_bytes()+
           network_Y2_matrix.get_memory_usage_in_bytes()+
           network_Y0_matrix.get_memory_usage_in_bytes()+

           network_Z1_matrix.get_memory_usage_in_bytes()+
           network_Z2_matrix.get_memory_usage_in_bytes()+
           network_Z0_matrix.get_memory_usage_in_bytes()+

           network_Y1_matrix.get_memory_usage_in_bytes()+

           network_BP_matrix.get_memory_usage_in_bytes()+
           network_BQ_matrix.get_memory_usage_in_bytes()+
           network_DC_B_matrix.get_memory_usage_in_bytes()+

           this_jacobian.get_memory_usage_in_bytes()+

           inphno.get_table_size()*2*sizeof(unsigned int);
}
