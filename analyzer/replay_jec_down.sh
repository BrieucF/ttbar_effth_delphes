#!/usr/bin/env bash

# Jet energy scale down
cp mis_cards/replay_nominal.yml mis_cards/config_replay/replay_jec_down.yml
sed -i s/flatTrees_with_weight_v2/jecdown_weightProd_v2/g mis_cards/config_replay/replay_jec_down.yml
sed -i s/nominal/jec_down/g mis_cards/config_replay/replay_jec_down.yml
python/replay.py mis_cards/config_replay/replay_jec_down.yml
