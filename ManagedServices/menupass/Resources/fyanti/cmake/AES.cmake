function(add_aes_target target_name file outfile)
	add_custom_target(
		${target_name}
        DEPENDS "${outfile}"
		COMMENT "Generated AES encrypted file ${outfile}" 
	)

	add_custom_command(
		OUTPUT "${outfile}"
		COMMAND aes-tool -e "${file}" -o "${outfile}"
		DEPENDS "${file}"
		COMMENT "AES encrypting ${file}" 
	)
endfunction()
