add scroll screen

add shell

move print/ to graphics/


trenne userspace von kernelspace so mit sicherheit das
selsbt beim absturz vom userspace der kernel in ein art sicherheits modus geht den fehler sucht probiert ihn zu beheben
den userspace neustartet wenn es nicht klappt nach einem reboot fragt und ein automatischen reboot per acpi nach 10 sekunden
macht somit sollte das meiste repariert werden
sobald der userspace abstürzt sollte der memory teil für userspace weg sein
und für den kernel da sein der kernel soll sofort alles unnütze und alles was für
den userspace da ist deaktivieren und herunterfahren um sich selbst zu schützen
dabei aber wichtige sachen aktiv lassen
