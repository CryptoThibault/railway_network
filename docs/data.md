# Scenario data reference

Sources checked on 2026-10-28. `data.json` uses SI units throughout. Distances are
in m, times in s, mass in kg, speed in m/s, and forces in N. The current linear
resistance coefficient `friction` is in N·s/m (force = coefficient × speed).
Source speeds below retain km/h only to make conversions auditable: divide by 3.6.

## City centres

Use GeoNames populated-place points as representative city-centre markers for
all cities, not railway stations or administrative-area centroids. These are
consistent cartographic reference points, not surveyed definitions of city centres.
GeoNames is a shared gazetteer, not a national mapping authority; attribution:
[GeoNames, CC BY 4.0](https://www.geonames.org/about.html).

Project WGS 84 longitude/latitude into EPSG:3857 with radius R = 6378137 m:
`x = R × longitude`, `y = R × ln(tan(π/4 + latitude/2))`, angles in radians.
Round x and y to the nearest meter. See [PROJ Web Mercator](https://proj.org/en/stable/operations/projections/webmerc.html).
Use the same projection for the map background. Map distances are distorted;
simulation lengths must not be calculated from projected straight lines.

| City | Latitude (° N) | Longitude (° E) | GeoNames source |
| --- | ---: | ---: | --- |
| Paris | 48.85341 | 2.34880 | [2988507](https://www.geonames.org/2988507/paris.html) |
| Lyon | 45.74906 | 4.84789 | [2996944](https://www.geonames.org/2996944/lyon.html) |
| Marseille | 43.29695 | 5.38107 | [2995469](https://www.geonames.org/2995469/marseille.html) |
| Berlin | 52.52437 | 13.41053 | [2950159](https://www.geonames.org/2950159/berlin.html) |
| Frankfurt | 50.11552 | 8.68417 | [2925533](https://www.geonames.org/2925533/frankfurt-am-main.html) |
| Zurich | 47.36667 | 8.55000 | [2657896](https://www.geonames.org/2657896/zuerich.html) |
| Rome | 41.89193 | 12.51133 | [3169070](https://www.geonames.org/3169070/rome.html) |

## Railway corridors

Segments abstract existing railway corridors between the selected cities. They
can combine several lines and require transfers in real life. Straight connections
are schematic, not track geometry: e.g. Marseille–Rome follows the coast rather
than crossing the sea. Intermediate towns not in the scenario are omitted.

| Connection | Real corridor / evidence | Length (m) | Speed ceiling (m/s) |
| --- | --- | ---: | ---: |
| Paris–Lyon | [SNCF high-speed service](https://www.sncf-connect.com/en-en/train/route/paris/lyon) | 429000 | 83.333333 |
| Lyon–Marseille | [SNCF high-speed service](https://www.sncf-connect.com/en-en/train/route/lyon/marseille) | 325000 | 83.333333 |
| Paris–Frankfurt | Saarbrücken–Mannheim, [DB/SNCF service](https://www.sncf-connect.com/en-en/train/route/paris/frankfurt-am-main) | 620000 | 88.888889 |
| Paris–Zurich | Dijon–Mulhouse–Basel, [TGV Lyria](https://www.tgv-lyria.com/fr/en/booking/destinations/paris-zurich) | 660000 | 88.888889 |
| Lyon–Zurich | Geneva–Lausanne–Bern, [SNCF connecting services](https://www.sncf-connect.com/en-en/train/route/lyon/zurich) | 450000 | 55.555556 |
| Frankfurt–Zurich | Mannheim–Karlsruhe–Basel, [DB services](https://www.bahn.de/reisen/view/verbindung/frankfurt/zuerich.shtml) | 460000 | 69.444444 |
| Frankfurt–Berlin | Erfurt–Halle, [DB ICE Sprinter](https://int.bahn.de/en/trains/long-distance-trains/ice_sprinter) | 650000 | 83.333333 |
| Zurich–Rome | Gotthard–Milan–Bologna–Florence, [SBB, transfer in Milan](https://www.sbb.ch/en/leisure-holidays/europe/countries-cities/italy/zurich-rome.html) | 900000 | 83.333333 |
| Lyon–Rome | Modane–Turin–Milan–Bologna–Florence, [Trenitalia connections](https://www.trenitalia.com/it/frecciarossa/collegamenti-frecciarossa/il-frecciarossa-arriva-a-parigi.html) | 1050000 | 83.333333 |
| Marseille–Rome | Nice–Ventimiglia–Genoa, [FS coastal itinerary](https://www.fstrenituristici.it/content/fs_treni_turistici/eng/destinations/EspressoRiviera.html) | 1000000 | 44.444444 |

Only Paris–Lyon and Lyon–Marseille lengths come from published railway distances
([SNCF methodology, p. 8](https://www.sncf-voyageurs.com/medias-publics/2025-09/sncf_voyageurs_methodologie-generale-infoges_v2025.pdf)).
Other lengths are rounded scenario estimates of the indicated corridors, not
verified track measurements or the straight-line distances on booking pages.

Speed ceilings use a documented maximum on a portion of a corridor, not a uniform
real-world speed limit or an average speed. The simulation currently applies each
ceiling to the entire segment; realistic speed profiles require finer segmentation.

- Paris–Lyon: 300 km/h, [SNCF Réseau infrastructure reference](https://www.sncf-reseau.com/medias-publics/2024-01/version_comparee_drr_2023_v3_et_drr_2023_v4.pdf).
  Lyon–Marseille: 300 km/h, [Méditerranée construction account](https://www.infociments.fr/sites/default/files/article/fichier/CM-94_0.pdf).
- Paris–Frankfurt: 320 km/h on [LGV Est](https://www.sncf-reseau.com/fr/a/trafic-reprend-sur-lgv-est).
  Paris–Zurich: 320 km/h, [TGV Lyria](https://www.tgv-lyria.com/ch/en/train-travel/onboard-services/wifi).
- Lyon–Zurich: 200 km/h on Mattstetten–Rothrist on the route via Bern,
  [SBB technical reference, p. 7](https://company.sbb.ch/content/dam/internet/corporate/downloads/de/sbb-als-geschaeftspartner/flotte-unterhalt/onestopshop/SA-NBS_SiKo.pdf.sbbdownload.pdf).
- Frankfurt–Zurich: 250 km/h on upgraded Rhine railway sections, not the entire
  route; [DB reports 67 km upgraded by 2026](https://www.deutschebahn.com/de/presse/pressestart_zentrales_uebersicht/presseinformationen-6868410).
- Frankfurt–Berlin: 300 km/h on Erfurt–Halle, [infrastructure contractor PORR](https://www.porr.de/projekte/detail/schnellfahrstrecke-erfurt-leipzig/halle-vde-82.pdf).
- Zurich–Rome and Lyon–Rome: 300 km/h on [Milan–Bologna, RFI](https://www.rfi.it/it/news-e-media/comunicati-stampa-e-news/2025/8/18/rfi---riaperta-questa-mattina-l-intera-linea-av-bologna-milano.html).
- Marseille–Rome: 160 km/h is a conservative scenario assumption based on the
  [Marseille–Ventimiglia section](https://ertms.be/members/sncf-france-2).
  The maximum across the whole coastal corridor has not been verified.

Existing infrastructure does not guarantee a current direct service: the
[FS Marseille–Rome night train is suspended in 2026](https://www.fstrenituristici.it/it/info-e-comunicazioni.html).

The selected straight segments have no intersections except shared city endpoints.
Paris–Marseille, Paris–Berlin and Frankfurt–Marseille are represented through
intermediate selected cities. Lyon–Frankfurt is omitted because its straight
connection crosses Paris–Zurich; Marseille–Zurich is omitted because it crosses
Lyon–Rome. This is a noncrossing subset of real corridors, not the complete railway
network. Additional cities or curved geometry will allow more connections later.

## Example services and routing

The twelve services in `data.json` are fictional simulation schedules, not
published timetables. Departures span 06:00–22:00; late services may arrive the
following day. Examples include Paris–Lyon without intermediate cities and
Berlin–Marseille via Frankfurt, Paris, and Lyon.

Dijkstra minimizes estimated elapsed seconds. Each segment starts and ends at
rest. Let `v = min(segment maxSpeed, train maxSpeed)`, `a = engineForce / mass`,
and `b = brakeForce / mass`. Accelerating and braking at v requires
`d = v²/(2a) + v²/(2b)` meters. For segment length L:

- If `L >= d`, duration is `v/a + (L - d)/v + v/b` seconds.
- Otherwise, peak speed is `sqrt(2L / (1/a + 1/b))`; duration is `peak/a + peak/b`.

Add `pause` once per intermediate selected city, never at the origin or final
destination. Acceleration, braking, and stop count can therefore change the chosen
route even with a zero pause. The exact selected segments are retained, including
parallel connections. A single-segment service has no intermediate stops in this
model, even if the abstract real corridor does.

This estimate assumes constant engine and brake forces and ignores linear
resistance and the fixed-step controller's integration and stopping tolerance.
Actual motion still applies resistance. Routes are fixed at initialization and do
not account for traffic, transfers, or time-dependent restrictions. Equal-cost
paths have no specified preference.

## Model and validation

`Train` owns its name and daily schedule; `TrainParameters` contains physical
properties only. `Board` holds location and the selected segments. The obsolete
Journey model and shared umbrella header have been removed; project headers
include their direct dependencies. Library utilities remain unchanged.

Loading requires an object with arrays of station, segment, and train objects.
Required numeric fields must be JSON numbers, not strings or booleans, and finite.
Names must be nonempty, have no control characters or surrounding whitespace.
Station names are unique and case-sensitive; segment and train station references
must resolve. EPSG:3857 coordinates must lie in the standard Web Mercator square
`[-20037508.342789244, 20037508.342789244]` m on both axes. Segments need distinct
endpoints, positive length, and positive speed. Mass, speed, forces, and train
length must be positive; resistance and pause can be zero. Departure is an integer
in `[0, 86400)` s. Train names may repeat; log filenames still distinguish collisions.

## States and events

`Scheduled` means awaiting the daily departure, `Waiting` is an intermediate
station pause, and `Completed` means the final destination has been reached.
Movement uses `Accelerating`, `Cruising`, and `Braking`. Only completed services
can reset; reset restores `Scheduled` and clears movement counters.

Each service emits `Departure`, `StationStop`, and `Arrival` events. Events carry
the train and station, time, and whether a departure begins a new daily journey.
The network coordinator timestamps them on the shared clock. `getEvents()` exposes
only events from the latest update; consume them before the next update. Borrowed
train and station pointers remain valid only while their registry objects live.
Printer handles console formatting; TrainLog uses events for journey boundaries
and states for periodic samples. Neither infers lifecycle from location changes.

## Daily operation and logs

The shared clock uses absolute simulated seconds; display includes a 1-based day
and time of day. Only `departure` is a daily time. Completed services wait at their
destination, then reset to the origin at the next scheduled departure. Speed,
travelled distance, segment progress, and intermediate pause counters reset for
that run. No return journey or physical repositioning is modelled. Midnight does
not reset active journeys. Departures missed while travelling are skipped; after
arrival, the next departure must be strictly later than arrival.

The simulation and schedule logic do not sleep or write files. The console entry
point controls real-time playback and cancellation; TrainLog manages one open
stream per train. The log directory is deleted and recreated before opening
the files; each simulation starts with fresh logs. `make clean` also removes it.
Filenames use sanitized train names, with ID suffixes if names collide. Records use SI units and cover
departure through arrival only, including
intermediate station pauses. Departures and arrivals are always recorded, even
between sampling boundaries; other records fall on five-minute clock boundaries.
No records are written while waiting for the first or next daily departure.
Day numbering restarts on each program launch.
Writing or opening errors stop the run with an error message.

The existing Logger writes through the global ThreadSafeIOStream output. Its
`setOutput` changes a shared stream pointer rather than opening a file, but every
insertion takes a mutex and flushes the stream. TrainLog avoids redirecting that
global output and builds a complete record before writing and flushing it once.
This keeps console events separate and leaves `lib/` unchanged.
