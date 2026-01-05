execute if score interaction bordertime matches 0 run summon interaction ~ ~-2 ~ {Tags:["poop"]}
execute if score interaction bordertime matches 0 run scoreboard players add interaction bordertime 1

scoreboard players add hours bordertime 1
execute if score hours bordertime matches 1 run schedule function namespace:stopsoundearly 60
execute if score hours bordertime matches 2.. run schedule function namespace:stopsoundearly 3
execute if score hours bordertime matches 2.. run tellraw @a [{"text":"Worldborder expanded for ","color":"green"},{"score":{"name":"hours","objective":"bordertime"}},{"text":" hours","color":"green"}]
execute if score hours bordertime matches ..1 run tellraw @a {"text":"Worldborder expanded for 1 hour","color":"green"}
execute if score ticks bordertime matches 0 run scoreboard players add hours bordertime 1
particle dust{color:[0.2,0.0,0.2],scale:4} ~ ~ ~ 0.5 0.5 0.5 0.1 1000 normal
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.5
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.51
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.52
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.53
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.54
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.55
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.56
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.57
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.58
playsound minecraft:item.goat_horn.sound.3 neutral @a ~ ~ ~ 1 0.59
kill @s
