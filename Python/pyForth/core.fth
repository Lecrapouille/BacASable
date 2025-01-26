: \ 10 WORD DROP ; IMMEDIATE

\ Nous venons d'implémenter les commentaires de ligne avec \
\ Continuons avec les commentaires multi-ligne.

\ Implémentation des commentaires multi-ligne Forth.
\ Le mot ( est immédiat et consomme tous les caractères jusqu'à
\ rencontrer le caractère ')' dont le code ASCII est 41.
\ Il faut impérativement que le mot ( soit séparé par des espaces
\ afin qu'il soit considéré comme un mot Forth par l'interpréteur.
\ Forth ne supporte pas les commentaires imbriqués.
\ WORD retourne la position du flux, information inutile et donc
\ éliminée par le mot DROP.

: ( 41 WORD DROP ; IMMEDIATE  \ Lire jusqu'à ) et ignorer le résultat

\ Exemples d'utilisation :
: TEST ( n -- n*2 ) DUP + . ; 2 TEST
