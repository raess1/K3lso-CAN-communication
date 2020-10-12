import keyboard

while True:
if keyboard.read_key() == "i":
{
	package[1] = package[1] + 1;
	if (package[1] == 95.5)	{
	package[1] = 94.5;
	}
}

if keyboard.read_key() == "k":
{
	package[1] = package[1] - 1;
	if (package[1] == -95.5)	{
	package[1] = -94.5;
	}
}

