#!/usr/bin/env bash

# Muon ISO reweighting down
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_muiso_down.yml
# modifies the config file for systematic computation
# modifies the output names 
sed -i s/nominal/muiso_down/g mis_cards/config_replay/replay_muiso_down.yml
#modifies the weight toward systematic
sed -i 's/llidiso/llidiso_muisodown/g' mis_cards/config_replay/replay_muiso_down.yml
#remove data since we have no data for systematics
sed -i '/Data_Run2015D-16Dec2015-v2_2016-03-03/,+7d' mis_cards/config_replay/replay_muiso_down.yml
python/replay.py mis_cards/config_replay/replay_muiso_down.yml

# treat the same flavor

cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_muiso_down_sf.yml
# modifies the config file for systematic computation
# modifies the output names 
sed -i s/nominal/muiso_down_sf/g mis_cards/config_replay/replay_muiso_down_sf.yml
#modifies the weight toward systematic
sed -i 's/llidiso/llidiso_muisodown/g' mis_cards/config_replay/replay_muiso_down_sf.yml
#remove data since we have no data for systematics
sed -i '/Data_Run2015D-16Dec2015-v2_2016-03-03/,+7d' mis_cards/config_replay/replay_muiso_down_sf.yml
#select only part of the events
sed -i 's/event_weight/event_weight\*(isElEl||isMuMu)/g' mis_cards/config_replay/replay_muiso_down_sf.yml
python/replay.py mis_cards/config_replay/replay_muiso_down_sf.yml

#treat the different flavor

cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_muiso_down_df.yml
# modifies the config file for systematic computation
# modifies the output names 
sed -i s/nominal/muiso_down_df/g mis_cards/config_replay/replay_muiso_down_df.yml
#modifies the weight toward systematic
sed -i 's/llidiso/llidiso_muisodown/g' mis_cards/config_replay/replay_muiso_down_df.yml
#remove data since we have no data for systematics
sed -i '/Data_Run2015D-16Dec2015-v2_2016-03-03/,+7d' mis_cards/config_replay/replay_muiso_down_df.yml
#select only part of the events
sed -i 's/event_weight/event_weight\*(isMuEl||isElMu)/g' mis_cards/config_replay/replay_muiso_down_df.yml
python/replay.py mis_cards/config_replay/replay_muiso_down_df.yml
