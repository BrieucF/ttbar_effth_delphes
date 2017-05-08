#!/usr/bin/env bash

# Jet energy resolution up
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jer_up.yml
# modifies the config file for systematic computation
# modifies the input rootfiles 
sed -i s/flatTrees_with_weight_v2_nonInfPunderatedWeight/flatTrees_with_weight_v2_nonInfPunderatedWeight_jerup/g mis_cards/config_replay/replay_jer_up.yml
sed -i s/skim_addWeight_v2_histos/skim_addWeight_v2_jerup_histos/g mis_cards/config_replay/replay_jer_up.yml
# modifies the output names 
sed -i s/nominal/jer_up/g mis_cards/config_replay/replay_jer_up.yml
#remove data since we have no data for systematics
sed -i '/Data_Run2015D-16Dec2015-v2_2016-03-03/,+7d' mis_cards/config_replay/replay_jer_up.yml
python/replay.py mis_cards/config_replay/replay_jer_up.yml

# treat the same flavor

cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jer_up_sf.yml
# modifies the input rootfiles 
sed -i s/flatTrees_with_weight_v2_nonInfPunderatedWeight/flatTrees_with_weight_v2_nonInfPunderatedWeight_jerup/g mis_cards/config_replay/replay_jer_up_sf.yml
sed -i s/skim_addWeight_v2_histos/skim_addWeight_v2_jerup_histos/g mis_cards/config_replay/replay_jer_up_sf.yml
# modifies the output names 
sed -i s/nominal/jer_up_sf/g mis_cards/config_replay/replay_jer_up_sf.yml
#remove data since we have no data for systematics
sed -i '/Data_Run2015D-16Dec2015-v2_2016-03-03/,+7d' mis_cards/config_replay/replay_jer_up_sf.yml
#select only part of the events
sed -i 's/event_weight/event_weight\*(isElEl||isMuMu)/g' mis_cards/config_replay/replay_jer_up_sf.yml
python/replay.py mis_cards/config_replay/replay_jer_up_sf.yml

#treat the different flavor

cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jer_up_df.yml
# modifies the input rootfiles 
sed -i s/flatTrees_with_weight_v2_nonInfPunderatedWeight/flatTrees_with_weight_v2_nonInfPunderatedWeight_jerup/g mis_cards/config_replay/replay_jer_up_df.yml
sed -i s/skim_addWeight_v2_histos/skim_addWeight_v2_jerup_histos/g mis_cards/config_replay/replay_jer_up_df.yml
# modifies the output names 
sed -i s/nominal/jer_up_df/g mis_cards/config_replay/replay_jer_up_df.yml
#remove data since we have no data for systematics
sed -i '/Data_Run2015D-16Dec2015-v2_2016-03-03/,+7d' mis_cards/config_replay/replay_jer_up_df.yml
#select only part of the events
sed -i 's/event_weight/event_weight\*(isMuEl||isElMu)/g' mis_cards/config_replay/replay_jer_up_df.yml
python/replay.py mis_cards/config_replay/replay_jer_up_df.yml
