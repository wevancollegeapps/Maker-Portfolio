execute as @e[type=item, name="Repeating Command Block"] at @s if block ~ ~-0.125 ~ structure_block run function namespace:addhour
execute as @e[type=item, name="Worldborder Expander"] at @s if block ~ ~-0.125 ~ structure_block run function namespace:addhour

execute as @e[type=item, name="Command Block"] at @s if block ~ ~-0.125 ~ structure_block run particle dust{color:[0.2,0.0,0.0],scale:4} ~ ~ ~ 0.1 0 0.1 0.01 1 normal
execute as @e[type=item, name="Worldborder Expansion Piece"] at @s if block ~ ~-0.125 ~ structure_block run particle dust{color:[0.2,0.0,0.0],scale:4} ~ ~ ~ 0.1 0 0.1 0.01 1 normal

execute as @e[type=item, name="Command Block"] at @s if block ~ ~-0.125 ~ structure_block run title @a[distance=..10] actionbar {"text":"four crafted pieces needed","color":"red"}
execute as @e[type=item, name="Worldborder Expansion Piece"] at @s if block ~ ~-0.125 ~ structure_block run title @a[distance=..10] actionbar {"text":"four crafted pieces needed","color":"red"}

execute store result score @a repeatingcommandblock run clear @a minecraft:repeating_command_block 0
execute as @a[scores={repeatingcommandblock=2..}] run function namespace:morethan1expander

execute if score ticks bordertime matches 1.. run scoreboard players remove ticks bordertime 1
execute if score ticks bordertime matches 0 run execute if score hours bordertime matches 1.. run scoreboard players remove hours bordertime 1
execute if score ticks bordertime matches 0 run execute if score hours bordertime matches 1.. run scoreboard players add ticks bordertime 72000

execute if score hours bordertime matches 1.. run worldborder set 8192
execute if score hours bordertime matches ..0 run worldborder set 512

execute if score interaction bordertime matches 1 run function namespace:actionbartimer

scoreboard players add trackingtick bordertime 1
execute if score trackingtick bordertime matches 20 run scoreboard players set trackingtick bordertime 0


