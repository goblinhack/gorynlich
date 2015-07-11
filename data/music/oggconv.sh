for i in DST_Alters.wav DST_AncientCurse_II.wav DST_Arch_Delerium.wav DST_Arches.wav DST_BattleLands.wav DST_BlackRiver.wav DST_BoneTower.wav DST_ClockTower.wav DST_DarkestKnight.wav DST_DoomCity.wav DST_ExInfernus.wav DST_LeDanse.wav DST_Legends.wav DST_SuperWin.wav charlotte.wav charlotte8bit.wav charlotte_slow.wav 
do
f=`echo $i | sed 's/\..*//g'`
echo $f
oggenc -q 10 $f.wav -o $f.ogg
done
