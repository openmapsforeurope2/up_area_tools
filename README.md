# up_area_tools

## Context: Ome2


## Description


## Configuration


## Utilisation

Paramètres :

* c [obligatoire] : chemin vers le fichier de configuration
* T [obligatoire] : thème (doit être parmi les valeurs : tn, hy, au, ib)
* op [obligatoire] : opération (doit être parmi les valeurs : create, extract)
* f [obligatoire] : feature à traiter
* cc [obligatoire] : code pays simple

Paramètres spécifiques à l'opération 'extract' :

* ncc [optionnel] : code pays voisin
* from [obligatoire] : thème source (doit être parmi les valeurs : prod, ref, up, work)
* to [obligatoire] : thème cible (doit être parmi les valeurs : prod, ref, up, work)
* no_reset [optionnel] : indique si la table cible doit être vidée ou non avant de réaliser l'extraction
* without_ids [optionnel] : indique si les identifiants des objets extraits doivent être enregistrés dans la table des identifiants


Exemples d'appels:

~~~
bin/up_area_tools --c path/to/config/epg_parmaters.ini --op extract --T tn --f road_link --cc lu --from up --without_ids 
bin/up_area_tools --c path/to/config/epg_parmaters.ini --op extract --T tn --f road_link --cc lu --from up --without_ids --ncc be
bin/up_area_tools --c path/to/config/epg_parmaters.ini --op create --T tn --f road_link --cc lu
~~~