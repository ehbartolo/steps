#include "header/apis/steps_api.h"
#include "header/basic/utility.h"
#include "header/steps_namespace.h"
#include "header/data_imexporter/psse_imexporter.h"
#include "header/data_imexporter/bpa_imexporter.h"

int api_get_source_integer_data(size_t bus, char* identifier, char* parameter_name)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);
        if(PARAMETER_NAME=="BUS" or PARAMETER_NAME=="BUS NUMBER")
            return sourceptr->get_source_bus();

        if(PARAMETER_NAME=="BUS_REG" or PARAMETER_NAME=="BUS TO REGULATE")
            return sourceptr->get_bus_to_regulate();

        if(PARAMETER_NAME=="MODE" or PARAMETER_NAME=="REGULATING MODE")
        {
            SOURCE_REGULATING_MODE mode = sourceptr->get_regulating_mode();
            if(mode==REGULATING_PQ)
                return 1;
            else
            {
                if(mode==REGULATING_PV)
                    return 2;
                else
                    return 3;
            }
        }

        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
        return 0;
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when retrieving data with api "<<__FUNCTION__<<endl
               <<"0 will be returned.";
        show_information_with_leading_time_stamp(sstream);
        return 0;
    }
}

void api_set_source_integer_data(size_t bus, char* identifier, char* parameter_name, int value)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);
        if(PARAMETER_NAME=="BUS" or PARAMETER_NAME=="BUS NUMBER")
            return sourceptr->set_source_bus(value);

        if(PARAMETER_NAME=="BUS_REG" or PARAMETER_NAME=="BUS TO REGULATE")
            return sourceptr->set_bus_to_regulate(value);

        if(PARAMETER_NAME=="MODE" or PARAMETER_NAME=="REGULATING MODE")
        {
            if(value==1)
                return sourceptr->set_regulating_mode(REGULATING_PQ);
            else
            {
                if(value==2)
                    return sourceptr->set_regulating_mode(REGULATING_PV);
                else
                {
                    if(value==3)
                        return sourceptr->set_regulating_mode(REGULATING_VA);
                    else
                    {
                        ostringstream sstream;
                        sstream<<"Input regulating mode "<<value<<" is not supported for PARAMETER_NAME=="<<sourceptr->get_device_name()<<" with api "<<__FUNCTION__<<endl
                               <<"Nothing will be set.";
                        show_information_with_leading_time_stamp(sstream);
                        return;
                    }
                }
            }
        }
        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when changing data with api "<<__FUNCTION__<<endl
               <<"Nothing will be set.";
        show_information_with_leading_time_stamp(sstream);
    }
}


double api_get_source_float_data(size_t bus, char* identifier, char* parameter_name)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);
        if(PARAMETER_NAME=="MBASE_MVA" or PARAMETER_NAME=="MBASE IN MVA")
            return sourceptr->get_mbase_in_MVA();

        if(PARAMETER_NAME=="PGEN_MW" or PARAMETER_NAME=="ACTIVE POWER GENERATION IN MW")
            return sourceptr->get_p_generation_in_MW();

        if(PARAMETER_NAME=="QGEN_MVAR" or PARAMETER_NAME=="REACTIVE POWER GENERATION IN MVAR")
            return sourceptr->get_q_generation_in_MVar();

        if(PARAMETER_NAME=="PMAX_MW" or PARAMETER_NAME=="MAX ACTIVE POWER GENERATION IN MW")
            return sourceptr->get_p_max_in_MW();

        if(PARAMETER_NAME=="PMIN_MW" or PARAMETER_NAME=="MIN ACTIVE POWER GENERATION IN MW")
            return sourceptr->get_p_min_in_MW();

        if(PARAMETER_NAME=="QMAX_MVAR" or PARAMETER_NAME=="MAX REACTIVE POWER GENERATION IN MVAR")
            return sourceptr->get_q_max_in_MVar();

        if(PARAMETER_NAME=="QMIN_MVAR" or PARAMETER_NAME=="MIN REACTIVE POWER GENERATION IN MVAR")
            return sourceptr->get_q_min_in_MVar();

        if(PARAMETER_NAME=="VREG_PU" or PARAMETER_NAME=="VOLTAGE TO REGULATE IN PU")
            return sourceptr->get_voltage_to_regulate_in_pu();

        if(PARAMETER_NAME=="RSOURCR_PU" or PARAMETER_NAME=="SOURCE RESISTANCE IN PU")
            return sourceptr->get_source_impedance_in_pu().real();

        if(PARAMETER_NAME=="XSOURCE_PU" or PARAMETER_NAME=="SOURCE REACTANCE IN PU")
            return sourceptr->get_source_impedance_in_pu().imag();

        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
        return 0.0;
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when retrieving data with api "<<__FUNCTION__<<endl
               <<"0.0 will be returned.";
        show_information_with_leading_time_stamp(sstream);
        return 0.0;
    }
}

void api_set_source_float_data(size_t bus, char* identifier, char* parameter_name, double value)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);

        if(PARAMETER_NAME=="MBASE_MVA" or PARAMETER_NAME=="MBASE IN MVA")
            return sourceptr->set_mbase_in_MVA(value);

        if(PARAMETER_NAME=="PGEN_MW" or PARAMETER_NAME=="ACTIVE POWER GENERATION IN MW")
            return sourceptr->set_p_generation_in_MW(value);

        if(PARAMETER_NAME=="QGEN_MVAR" or PARAMETER_NAME=="REACTIVE POWER GENERATION IN MVAR")
            return sourceptr->set_q_generation_in_MVar(value);

        if(PARAMETER_NAME=="PMAX_MW" or PARAMETER_NAME=="MAX ACTIVE POWER GENERATION IN MW")
            return sourceptr->set_p_max_in_MW(value);

        if(PARAMETER_NAME=="PMIN_MW" or PARAMETER_NAME=="MIN ACTIVE POWER GENERATION IN MW")
            return sourceptr->set_p_min_in_MW(value);

        if(PARAMETER_NAME=="QMAX_MVAR" or PARAMETER_NAME=="MAX REACTIVE POWER GENERATION IN MVAR")
            return sourceptr->set_q_max_in_MVar(value);

        if(PARAMETER_NAME=="QMIN_MVAR" or PARAMETER_NAME=="MIN REACTIVE POWER GENERATION IN MVAR")
            return sourceptr->set_q_min_in_MVar(value);

        if(PARAMETER_NAME=="VREG_PU" or PARAMETER_NAME=="VOLTAGE TO REGULATE IN PU")
            return sourceptr->set_voltage_to_regulate_in_pu(value);

        if(PARAMETER_NAME=="RSOURCE_PU" or PARAMETER_NAME=="SOURCE RESISTANCE IN PU")
        {
            complex<double> Z = sourceptr->get_source_impedance_in_pu();
            return sourceptr->set_source_impedance_in_pu(complex<double>(value, Z.imag()));
        }
        if(PARAMETER_NAME=="XSOURCE_PU" or PARAMETER_NAME=="SOURCE REACTANCE IN PU")
        {
            complex<double> Z = sourceptr->get_source_impedance_in_pu();
            return sourceptr->set_source_impedance_in_pu(complex<double>(Z.real(), value));
        }
        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when changing data with api "<<__FUNCTION__<<endl
               <<"Nothing will be set.";
        show_information_with_leading_time_stamp(sstream);
    }
}

const char* api_get_source_string_data(size_t bus, char* identifier, char* parameter_name)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    string BLANK = "";

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);
        if(PARAMETER_NAME=="ID" or PARAMETER_NAME=="IDENTIFIER")
            return sourceptr->get_identifier().c_str();

        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
        return BLANK.c_str();
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when retrieving data with api "<<__FUNCTION__<<endl
               <<"EMPTY STRING will be returned.";
        show_information_with_leading_time_stamp(sstream);
        return BLANK.c_str();
    }
}


void api_set_source_string_data(size_t bus, char* identifier, char* parameter_name, char* value)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);
        if(PARAMETER_NAME=="ID" or PARAMETER_NAME=="IDENTIFIER")
            return sourceptr->set_identifier(value);

        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
        return;
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when changing data with api "<<__FUNCTION__<<endl
               <<"Nothing will be set.";
        show_information_with_leading_time_stamp(sstream);
    }
}

bool api_get_source_boolean_data(size_t bus, char* identifier, char* parameter_name)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);

        if(PARAMETER_NAME=="STATUS")
            return sourceptr->get_status();

        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
        return false;
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when retrieving data with api "<<__FUNCTION__<<endl
               <<"False will be returned.";
        show_information_with_leading_time_stamp(sstream);
        return false;
    }
}


void api_set_source_boolean_data(size_t bus, char* identifier, char* parameter_name, bool value)
{
    DEVICE_ID generator_did, pe_source_did;
    generator_did = get_generator_device_id(bus, identifier);
    pe_source_did = get_pe_source_device_id(bus, identifier);

    POWER_SYSTEM_DATABASE* psdb = get_default_power_system_database();

    SOURCE* sourceptr = NULL;
    sourceptr = psdb->get_source(generator_did);
    if(sourceptr==NULL)
        sourceptr = psdb->get_source(pe_source_did);

    if(sourceptr!=NULL)
    {
        string PARAMETER_NAME = string2upper(parameter_name);

        if(PARAMETER_NAME=="STATUS")
            return sourceptr->set_status(value);

        show_parameter_not_supported_for_device_with_api(PARAMETER_NAME, sourceptr->get_device_id(), __FUNCTION__);
        return;
    }
    else
    {
        ostringstream sstream;
        sstream<<"Neither "<<generator_did.get_device_name()<<" nor PARAMETER_NAME=="<<pe_source_did.get_device_name()<<" exists in database when changing data with api "<<__FUNCTION__<<endl
               <<"Nothing will be set.";
        show_information_with_leading_time_stamp(sstream);
    }
}
