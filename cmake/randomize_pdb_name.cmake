function(RANDOMIZE_PDB_NAME target)
    if (NOT ALCHEMY_NO_HOT_RELOAD)
        string(RANDOM random_string)
        set(pdbname "${target}_${random_string}")
        set_target_properties(${target} PROPERTIES
            COMPILE_PDB_NAME ${pdbname}
            PDB_NAME ${pdbname})
    endif()
endfunction()
