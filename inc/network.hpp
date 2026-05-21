#pragma once
#include "libftpp.hpp"
#include "segment.hpp"
#include "station.hpp"
#include "train.hpp"
#include "train_type.hpp"
#include "journey.hpp"

#define LOG_LEVEL Debug

void factory_init();
void registry_init();
void logger_init();

void printStations();
void printSegments();
void printTrains();