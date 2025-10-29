
- modifier le Interface-Framework pour qu'il n'utilise plus 
  "Processor yield" mais des semaphore signalées en cas d'input
  utilisateur. par ex, objet global 
  "EventNotifier onEvent: anEventType signal: aSemaphore". Ça permet
  à tous les "Controller" d'être notifié, et de déterminer si l'evenement
  les concernent.

- modifier ProcessScheduler 
  . pour partager les ressources automatiquement
  . pour ne plus utiliser de backgroundProcess

- modifier l'interpreteur pour se mettre en pose quand tous les procs
  attendent un evenement.

- modifier les classes dans "Graphics-..." pour le remplacer par un
  système qui délégue un max de taches à SDL.
  Voir:
    . pharo-graphics/Bloc
    . pharo-graphics/Toplo
