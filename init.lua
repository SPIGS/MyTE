function Test()
	print("Hello world from lua")
end

Myte.register("Test")
Myte.bind("f1", "Test")
Myte.bind("f2", "splat")

Myte.bind("left", "moveCursorLeft")
Myte.bind("right", "moveCursorRight")
Myte.bind("up", "moveCursorUp")
Myte.bind("down", "moveCursorDown")
Myte.bind("backspace", "deleteGraphemeLeft")
Myte.bind("delete", "deleteGraphemeRight")
Myte.bind("ctrl + right", "moveCursorEndOfNextWord")
Myte.bind("ctrl + left", "moveCursorBegOfPrevWord")
Myte.bind("ctrl + backspace", "deleteWordLeft")
Myte.bind("ctrl + delete", "deleteWordRight")
Myte.bind("ctrl + p", "openCommandModal")
Myte.bind("ctrl + s", "save")
Myte.bind("ctrl + shift + s", "saveAs")
Myte.bind("ctrl + v", "paste")

Myte.a = 69
Myte.b = 4.20
Myte.c = "This is read from the config!"
Myte.d = true
