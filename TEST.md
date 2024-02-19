## Testing overrides

Some test behaviors are controlled by test overrides set via environment variables.
They only work when the CMake Option SFS_ENABLE_TEST_OVERRIDES is set to "On".
To enable that during build, you must use the switch "-EnableTestOverrides" ("--enable-test-overrides" on Linux) with the build scripts.

If you're using the CMake extension on VSCode, you can set the build options through the VSCode settings. Add the below to either your user or workspace JSON settings.

```json
"cmake.configureArgs": [
    "-DSFS_CLIENT_TEST_OVERRIDES=ON"
]
```

When these test overrides are enabled, a few environment variables can be used to adjust the behavior of the tool:

| Environment Variable                          | Description                                                                                 |
|-----------------------------------------------|---------------------------------------------------------------------------------------------|
| SFS_TEST_OVERRIDE_BASE_URL                    | Set this to any string value which will be used as the SFS URL rather than the default one. |
| SFS_TEST_OVERRIDE_NO_CONNECTION_CONFIG_LIMITS | Set this to remove the limitations on the values of ConnectionConfig.                       |
