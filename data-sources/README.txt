This is where we handle downloading, unpacking, etc the various external
(public) datasets.

All data sources should get their own directory and have their
non-distribution files (anything downloaded, etc) included in ../.gitignore
so that they never get accidentally uploaded.


Processing scripts belong here, but all sql should be in ../sql, please.
