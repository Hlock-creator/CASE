// PRINT
print "текст"


// ПЕРЕМЕННАЯ
set название = число/текст

// ВВОД ТЕКСТА ИЗ ТЕРМИНАЛА

ent название


// IF ELSE

if условие
то что нужно сделать
else (не обязательно)
то что нужно делать
endif


// МАТЕМАТИКА

math название_переменной_результата = число/переменная -+/* число/переменная


// ООП

Class название {
  set название = текст/число
}

new имя_класса имя_объекта

ПРИМЕР:

Class Player{
  set healt = 100
  set damage = 20
}

new Player hero (теперь у hero есть healt = 100 и damage = 20)
new Player enemy (теперь у enemy тоже есть healt = 100 и damage = 20)

math enemy.healt = enemy.healt - player.damage  (player бьет по enemy и enemy теряет ХП)
print enemy.healt (выводит что enemy.healt = 80)


// WHILE

while условие {
  то что нужно сделать
}

ПРИМЕР:

set a = 20

while a = 20 {
  print "Hello world"
}


// РАБОТА С ФАЙЛАМИ

write "имя_файла" текст  // создает файл и записывает в него текст/числа и т.д.

read "имя_файла" переменная  // читает что написано в файле и записывает содержимое в переменную

delete "имя_файла"  // удаляет файл

