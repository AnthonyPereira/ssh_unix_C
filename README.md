# Projet SE-Rés

## Pourcentage de travail
- Benoît Brebion : 50%
- Anthony Pereira : 50%

## Notes importantes
- Le Makefile devrait générer les 6 exécutables (mysh, myls, myps, myssh, mysshd et myssh-server) comme demandé. Ces 6 exécutables peuvent être lancés individuellement (mysh pour exécuter des commandes en local par exemple).
- Le mysshd et le myssh-server doivent être lancés manuellement dans 2 terminaux différents avant de lancer myssh.
- Le mysshd doit être lancé avec `sudo` (nous n'avons pas réussi à fixer le sticky bit).
- mysshd et myssh-server sont compatibles avec plusieurs clients connectés en même temps.
- mysshd et myssh-server doivent être fermés avec Ctrl-C.
- myssh affiche le retour des commandes effectuées dans le shell à distance, nous ne savions pas si il fallait implémenter cette fonctionnalité ou non.
- Pour les différentes commandes (contenant des séquencements par exemple) il est nécessaire de mettre des espaces (ex: `ls && ls` et non pas `ls&&ls`).
- Ctrl-C fonctionne dans mysh mais est considéré comme une demande de sortie du shell lorsqu'un processus est en cours d’exécution.
- Exemples pour les variables locales : `set a=foo`, `echo $a`, `unset $a`.
- Exemples pour les variables d'environnement : `setenv b=foo`, `echo $b`, `unsetenv $b` (si 1 variable d'environnement et 1 variable locale sont initialisées avec le même nom, le `echo` affichera en priorité celle d'environnement).
- Le projet a été développé et testé sous Ubuntu uniquement.

## Fonctionnalités implémentées
- mysh
    - 1. Lancement de commandes
        - a. Séquencement
    - 2. Commandes
        - a. Changer de répertoire
        - b. Sortie du shell (et propagation du CTRL+c)
        - c. Code de retour d’un processus
        - d. Lister le contenu d’un répertoire (myls)
        - e. Afficher l’état des processus en cours (myps)
    - 3. Les redirections
        - a. Les tubes
        - b. Redirections vers ou depuis un fichier (sauf `<`)
    - 5. Les variables
- myssh
    - 1. Identification
    - 2. Configuration
- mysshd
- myssh-server
- Protocole de communication SSH
    - 1. Généralité et format du protocole
    - 2. Authentification
        - a. Requête d’authentification
        - b. Réponse du serveur
    - 3. Envoi d’une commande
        - a. Cas numéro 1 (exec)
        - b. Cas numéro 2 (shell)

## Fonctionnalités non implémentées
- mysh
    - 1. Lancement de commandes
        - b. Wildcards
    - 4. Premier et arrière plans
        - a. Commande myjobs
        - b. Passer une commande de foreground à background et inversement
- myssh
    - 3. Sortie du shell et propagation du CTRL-C
- Protocole de communication SSH
    - 4. Envoi d’un signal

## Bugs
Les différentes fonctionnalités implémentées fonctionnent complètement. Nous reconnaissons qu'il y a cependant quelques bugs de mémoire résiduels que nous avons un peu tardés à résoudre, mais ceux-ci ne devraient pas géner à l'utilisation.  
Il est également possible que dans certains rares cas, certains cas d'erreurs n'aient pas été gérés. Nous avons cependant veillé à gérer au maximum ces cas d'erreurs. Cela vaut notamment pour la présence ou non d'espaces dans les commandes avec séquencement par exemple.

## Petite blague pour détendre l'atmosphère
Une requête TCP entre dans un bar et dit :  
- Je veux une bière !
- Vous voulez une bière ?
- Oui, je veux une bière !
- Très bien, je vous prépare une bière.
# ssh_unix_C
