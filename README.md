# area_matching

## Context

Open Maps For Europe 2 est un projet qui a pour objectif de développer un nouveau processus de production dont la finalité est la construction d'un référentiel cartographique pan-européen à grande échelle (1:10 000).

L'élaboration de la chaîne de production a nécessité le développement d'un ensemble de composants logiciels qui constituent le projet [OME2](https://github.com/openmapsforeurope2/OME2).


## Description

Cette application fournit des fonctions utilitaires qui impliquent les zones de mise à jour.

Dans le cadre de la mise à jour des données d'un pays le processus de mise en cohérence aux frontières ne sera pas joué sur l'ensemble des données transfrontalières mais uniquement dans des zones environnant les objets créés, supprimés ou modifiés, ce sont les zone de mise à jour.


## Fonctionnement

Deux traitements peuvent être réalisés grâce à cette application:

### 1. la génération des zones de mise à jour

Les zones de mise à jour sont générées par aggrégation des buffers réalisés autour des objets situés à proximité des frontières qui ont été créés, supprimés ou modifiés. Une donnée d'entrée est donc le résultat du calcul différentiel (table Postgresql) réalisé entre la nouvelle version N des données d'un pays avec la version antérieure N-1 de ces mêmes données.

Un multi-polygone est créé pour chaque frontière. On entend ici par frontière la limite entre un couple de pays. Pour un pays traité il y a donc autant de multi-polygones que de pays limitrophes.

### 2. l'extraction d'objets

L'extraction consiste à copier l'ensemble des objets d'une table source qui sont en intersection avec les zones de mise à jour vers une table cible.

Etant donné la taille importante que peuvent avoir les zones de mise à jour, il est possible de daller le calcul (tessellation par subdivision des multi-polygones en polygones de petites tailles) afin d'optimiser l'utilisation de la mémoire et d'accélérer le calcul.


## Configuration

Les paramètres de configuration permettent un réglage fin du comportement des algorithmes. Il est possible de définir un paramètrage différent pour chaque thème.

On trouve dans le [dossier de configuration](https://github.com/openmapsforeurope2/area_matching/tree/main/config) les fichiers suivants :
- epg_parameters.ini : regroupe des paramètres de base issus de la bibliothèque libepg qui constitue le socle de développement l'outil. Ce fichier est aussi le fichier chapeau qui pointe vers les autres fichiers de configurations.
- db_conf.ini : informations de connexion à la base de données.
- theme_parameters.ini : configuration des paramètres spécifiques à l'application.


## Utilisation

L'outil s'utilise en ligne de commande.

Paramètres communs à toutes les opérations :

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

<br>

Exemple d'appel pour lancer la génération des zones de mise à jour pour une classe d'objet (road_link) pour un pays (luxembourg) :
~~~
bin/up_area_tools --c path/to/config/epg_parmaters.ini --op create --T tn --f road_link --cc lu
~~~

Exemple d'appel pour lancer l'extraction des données depuis la table de mise à jour vers la table de travail :
~~~
bin/up_area_tools --c path/to/config/epg_parmaters.ini --op extract --T tn --f road_link --cc lu --from up --without_ids 
~~~

Exemple d'appel pour lancer l'extraction des données de deux pays (Belgique et Luxembourg) autour de leur frontière commune :
~~~
bin/up_area_tools --c path/to/config/epg_parmaters.ini --op extract --T tn --f road_link --cc lu --from up --ncc be
~~~