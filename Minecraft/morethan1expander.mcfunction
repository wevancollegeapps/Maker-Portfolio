clear @s repeating_command_block 1
execute at @s run summon minecraft:item ^ ^2 ^2 {Item:{id:"minecraft:repeating_command_block",count:1}}
execute at @s run particle minecraft:explosion ^ ^2 ^2
execute at @s run playsound minecraft:block.vault.close_shutter block @s ^ ^2 ^2