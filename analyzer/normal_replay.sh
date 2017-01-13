#!/usr/bin/env bash
# PU systematic
# more tricky due to 'pu' name which appear everywhere.. stars not supported also

python/replay.py mis_cards/replay_nominal.yml

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

