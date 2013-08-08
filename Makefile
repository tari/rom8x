.PHONY: 84CSE
84CSE: G84CSE1.8xp G84CSE2.8xp

G84CSE1.8xp: template.z80
	spasm -DprogName="\"84CSE\"" -DpageNum="\"1\"" '-DbootPage=$$FF' template.z80 G84CSE1.8xp
G84CSE2.8xp: template.z80
	spasm -DprogName="\"84CSE\"" -DpageNum="\"2\"" '-DbootPage=$$EF' template.z80 G84CSE2.8xp
