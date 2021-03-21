import pygame

pygame.init()
pygame.display.set_mode()   

def getKey():
    events = pygame.event.get()
    for event in events:
        if event.type == pygame.KEYDOWN:
            return event.key
    return False

while True:  # making a loop
    key = getKey()
    if key:
        print( getKey() )