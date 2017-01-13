#!/usr/bin/env bash

# Jet energy scale up
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jec_up.yml
sed -i s/flatTrees_with_weight_v2/jecup_weightProd_v2/g mis_cards/config_replay/replay_jec_up.yml
sed -i s/nominal/jec_up/g mis_cards/config_replay/replay_jec_up.yml
python/replay.py mis_cards/config_replay/replay_jec_up.yml
