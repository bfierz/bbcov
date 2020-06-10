# OpenCppCoverage BitBucket Server CodeCoverage Exporter

This exporter allows to generate line-coverage information for BitBucket Server according to this specification:
<https://marketplace.atlassian.com/apps/1218271/code-coverage-for-bitbucket-server?hosting=server&tab=overview>

## Build Instructions

The requirements are the same as for OpenCppCoverage. The repository contains a
simple VS 2017 solution in the folder BitBucketCodeCoverage.
Deploy the compiled plugin to the OpenCppCoverage plugin folder.

## Usage

Add the export statement to the command-line of OpenCppCoverage:

```cmd
--export_type BitBucketCodeCoverage:C:\path\to\root?output.json
```

* The name of the plugin is _BitBucketCodeCoverage_
* The first parameter after the `:` contains projects root folder. This is necessary as the BitBucket line coverage feature requires relatives paths
* The second optional parameter after the `?` denotes an optional name for the generated output file
