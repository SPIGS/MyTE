function Test()
	print("Hello world from lua")
end

Myte.register("Test")
Myte.bind("f1", "Test")
-- Myte.executeCommand("Test")
Myte.bind("f2", "splat")

Myte.bind("left", "moveCursorLeft")
Myte.bind("right", "moveCursorRight")
Myte.bind("backspace", "deleteGraphemeLeft")
Myte.bind("delete", "deleteGraphemeRight")

Myte.a = 69
Myte.b = 4.20
Myte.c = "This is read from the config!"
Myte.d = true
