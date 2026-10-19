#define AHCI_CAP        0x00    /* Host Capabilities */
#define AHCI_GHC        0x04    /* Global Host Control */
#define AHCI_IS         0x08    /* Interrupt Status */
#define AHCI_PI         0x0C    /* Ports Implemented (Bitmask of available ports) */
#define AHCI_VS         0x10    /* AHCI Version */
#define AHCI_CCC_CTL    0x14    /* Command Coalescing Control */
#define AHCI_CCC_PORTS  0x18    /* Command Coalescing Ports */
#define AHCI_EM_LOC     0x1C    /* Enclosure Management Location */
#define AHCI_EM_CTL     0x20    /* Enclosure Management Control */
#define AHCI_CAP2       0x24    /* Host Capabilities Extended */
#define AHCI_BOHC       0x28    /* BIOS/OS Handoff Control */

/* Useful Bits for Global Control */
#define GHC_AE          (1 << 31) /* AHCI Enable (Must be set to 1) */
#define GHC_IE          (1 << 1)  /* Interrupt Enable */
#define GHC_HR          (1 << 0)  /* HBA Reset */

/* Calculate the base memory address of a specific port */
#define AHCI_PORT_BASE(port)    (0x100 + ((port) * 0x80))

/* Port Specific Register Offsets */
#define AHCI_PxCLB      0x00    /* Command List Base Address (64-bit lower) */
#define AHCI_PxCLBU     0x04    /* Command List Base Address (64-bit upper) */
#define AHCI_PxFB       0x08    /* FIS Base Address (64-bit lower) */
#define AHCI_PxFBU      0x0C    /* FIS Base Address (64-bit upper) */
#define AHCI_PxIS       0x10    /* Interrupt Status */
#define AHCI_PxIE       0x14    /* Interrupt Enable */
#define AHCI_PxCMD      0x18    /* Command and Status */
#define AHCI_PxTFD      0x20    /* Task File Data (Status & Error bytes) */
#define AHCI_PxSIG      0x24    /* Signature (Device type tracking) */
#define AHCI_PxSSTS     0x28    /* Serial ATA Status (Physical link status) */
#define AHCI_PxSCTL     0x2C    /* Serial ATA Control */
#define AHCI_PxSERR     0x30    /* Serial ATA Error */
#define AHCI_PxSACT     0x34    /* Serial ATA Active (Used for NCQ) */
#define AHCI_PxCI       0x38    /* Command Issue (Flip bit to trigger command) */
#define AHCI_PXCMD_ST   (1u << 0)
#define AHCI_PXCMD_FRE  (1u << 4)
#define AHCI_PXCMD_FR   (1u << 14)
#define AHCI_PXCMD_CR   (1u << 15)


#define PORT_SSTS_DET_MASK      0x0F  /* Device Detection mask */
#define PORT_SSTS_DET_PRESENT   0x03  /* 0x3 = Device present and PHY communication established */

/* PxSIG (Device Signatures) values */
#define SATA_SIG_ATA            0x00000101  /* Standard SATA Hard Drive / SSD */
#define SATA_SIG_ATAPI          0xEB140101  /* CD/DVD/Blu-ray Optical Drive */
#define SATA_SIG_SEMB           0xC33C0101  /* Enclosure Management Bridge */
#define SATA_SIG_PM             0x96690101  /* Port Multiplier */

/* PxCMD (Command register bits) - used to spin up the port engines */
#define PORT_CMD_ST             (1 << 0)   /* Start processing command list */
#define PORT_CMD_FRE            (1 << 4)   /* FIS Receive Enable */
#define PORT_CMD_FR             (1 << 14)  /* FIS Receive Running status bit */
#define PORT_CMD_CR             (1 << 15)  /* Command List Running status bit */

/* ATA Command Codes */
#define ATA_CMD_IDENTIFY        0xEC       /* Identify Standard SATA Device */
#define ATA_CMD_IDENTIFY_PACKET 0xA1       /* Identify ATAPI (Optical) Device */

void achi_init(void);
