#!/usr/bin/env bash
# PU systematic
# more tricky due to 'pu' name which appear everywhere.. stars not supported also

# Nominal case with btagM

python/replay.py mis_cards/replay_nominal.yml

# treat the same flavor and different flavor
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_nominal_sf.yml
#ensure to have the correct naming for this particular replay
sed -i s/nominal/nominal_sf/g mis_cards/config_replay/replay_nominal_sf.yml
#select only part of the events
sed -i 's/event_weight/event_weight\*(isElEl||isMuMu)/g' mis_cards/config_replay/replay_nominal_sf.yml
python/replay.py mis_cards/config_replay/replay_nominal_sf.yml

# treat the different flavor and different flavor
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_nominal_df.yml
#ensure to have the correct naming for this particular replay
sed -i s/nominal/nominal_df/g mis_cards/config_replay/replay_nominal_df.yml
#select only part of the events
sed -i 's/event_weight/event_weight\*(isMuEl||isElMu)/g' mis_cards/config_replay/replay_nominal_df.yml
python/replay.py mis_cards/config_replay/replay_nominal_df.yml





#sed -i 's/jjbtag\*pu\*event_weight/jjbtag\*pu_up\*event_weight/g' mis_cards/config_replay/replay_nominal_sf.yml

#cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_pu_up.yml
#sed -i 's/jjbtag\*pu\*event_weight/jjbtag\*pu_up\*event_weight/g' mis_cards/config_replay/replay_pu_up.yml
#sed -i s/nominal/pu_up/g mis_cards/config_replay/replay_pu_up.yml
#python/replay.py mis_cards/config_replay/replay_pu_up.yml
#
#cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_pu_down.yml
#sed -i 's/jjbtag\*pu\*event_weight/jjbtag\*pu_down\*event_weight/g' mis_cards/config_replay/replay_pu_down.yml
#sed -i s/nominal/pu_down/g mis_cards/config_replay/replay_pu_down.yml
#python/replay.py mis_cards/config_replay/replay_pu_down.yml
#
# Jet energy scale 
#cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jec_up.yml
#sed -i s/flatTrees_with_weight_v2/jecup_weightProd_v2/g mis_cards/config_replay/replay_jec_up.yml
#sed -i s/nominal/jec_up/g mis_cards/config_replay/replay_jec_up.yml
#python/replay.py mis_cards/config_replay/replay_jec_up.yml
#
#cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jec_down.yml
#sed -i s/flatTrees_with_weight_v2/jecdown_weightProd_v2/g mis_cards/config_replay/replay_jec_down.yml
#sed -i s/nominal/jec_down/g mis_cards/config_replay/replay_jec_down.yml
#python/replay.py mis_cards/config_replay/replay_jec_down.yml

