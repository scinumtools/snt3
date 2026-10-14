#ifndef SNT_DIP_METADATA_FIELDS_H
#define SNT_DIP_METADATA_FIELDS_H

// Keep the metadata property inventory in one place so parsing, merging,
// serialization, and presence tracking cannot drift apart.
#define SNT_DIP_METADATA_SCALAR_FIELDS(X)                                                                              \
    X(Description, description, "description")                                                                         \
    X(Authors, authors, "authors")                                                                                     \
    X(Title, title, "title")                                                                                           \
    X(Journal, journal, "journal")                                                                                     \
    X(Year, year, "year")                                                                                              \
    X(Volume, volume, "volume")                                                                                        \
    X(Issue, issue, "issue")                                                                                           \
    X(Pages, pages, "pages")                                                                                           \
    X(DOI, doi, "doi")                                                                                                 \
    X(URL, url, "url")                                                                                                 \
    X(Version, version, "version")                                                                                     \
    X(Created, created, "created")                                                                                     \
    X(Modified, modified, "modified")                                                                                  \
    X(License, license, "license")                                                                                     \
    X(Rationale, rationale, "rationale")                                                                               \
    X(RecommendedRange, recommended_range, "recommended_range")                                                        \
    X(PerformanceImpact, performance_impact, "performance_impact")                                                     \
    X(ScientificImpact, scientific_impact, "scientific_impact")                                                        \
    X(Deprecated, deprecated, "deprecated")                                                                            \
    X(Replacement, replacement, "replacement")                                                                         \
    X(Since, since, "since")                                                                                           \
    X(Category, category, "category")                                                                                  \
    X(Visibility, visibility, "visibility")

#define SNT_DIP_METADATA_LIST_FIELDS(X)                                                                                \
    X(Native, native, "native")                                                                                        \
    X(Requires, requires, "requires")                                                                                  \
    X(Conflicts, conflicts, "conflicts")                                                                               \
    X(Implies, implies, "implies")                                                                                     \
    X(See, see, "see")                                                                                                 \
    X(Example, example, "example")

#endif
