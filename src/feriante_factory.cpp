#include "../includes/feriante_factory.h"

#include "../includes/feriante_estatico.h"
#include "../includes/feriante_estacional.h"
#include "../includes/feriante_estacional_proporcional.h"

FerianteFactory::FerianteFactory(FEL *_fel, Environment *_env, Monitor *_monitor, MercadoMayorista *_mer) : fel(_fel), env(_env), monitor(_monitor), mer(_mer)
{
}

Feriante *FerianteFactory::create_feriante(std::string const &feriante_type, const int feria_id, const int prod_amount)
{
    Feriante *feriante;

    if (feriante_type == "static")
    {
        feriante = new FerianteEstatico(this->fel, this->mer, feria_id);
    }
    else if (feriante_type == "seasonal")
    {
        feriante = new FerianteEstacional(this->fel, this->mer, feria_id);
    }
    else if (feriante_type == "seasonal-proportional")
    {
        feriante = new FerianteEstacionalProporcional(this->fel, this->mer, feria_id, prod_amount);
    }

    feriante->set_environment(this->env);
    feriante->set_monitor(this->monitor);

    return feriante;
}