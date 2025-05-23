# Page Rank

https://github.com/user-attachments/assets/e0b626cd-3cee-4101-8793-e1ebf7edc476

This project simulates the [page rank](https://en.wikipedia.org/wiki/PageRank) algorithm, notes:

- Each node represents a web page and its links, and each yellow sphere is a user that jumps between pages following the edges
- It simulate a scale-free graph by applying preferential attachment on nodes, creating hubs, similar to web.
- It has a "physics engine" where you can change settings like: gravity, attraction and spring force.
- For college project compliance it also has a fancy illumination setting.

# Running the system

Clone the project (including the submodules!):

```bash
git clone --recurse-submodules -j8 git@github.com:diegoreis42/page-rank.git

./scripts/build.sh # to build the project

./build/main # to run the project
```


This project was made by me and [diogo-ha](github.com/diogo-ha)
