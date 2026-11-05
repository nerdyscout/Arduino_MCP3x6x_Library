# Spec Delta

## Purpose

Define the metadata and documentation behavior expected from the library release artifacts for
the MCP3x6xR Arduino library.

## ADDED Requirements

### Requirement: Release metadata and documentation reflect corrected behavior and version

The system SHALL publish release metadata and documentation that identify version 0.2.0, SHALL
reference the GitHub repository, and SHALL document the corrected ADC behavior including the
voltage conversion and the upgrade path.

#### Scenario: Release version
- **WHEN** the release metadata is inspected
- **THEN** version 0.2.0 is declared

#### Scenario: Upgrade documentation
- **WHEN** the release documentation is inspected
- **THEN** the upgrade path to 0.2.0 is documented
