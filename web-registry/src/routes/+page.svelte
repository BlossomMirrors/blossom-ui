<script lang="ts">
	import { Button } from "$lib/registry/ui/button";
	import ModeToggle from "$lib/registry/ui/mode-toggle/mode-toggle.svelte";
	import { mode, toggleMode } from "mode-watcher";

	const variants = ["default", "primary", "destructive", "ghost", "link"] as const;
	const sizes = ["sm", "default", "lg", "icon"] as const;
</script>

<div class="mx-auto flex min-h-svh max-w-3xl flex-col gap-8 px-4 py-8">
	<header class="flex items-start justify-between gap-4">
		<div class="flex flex-col gap-1">
			<h1 class="text-3xl font-bold tracking-tight">Blossom UI Registry</h1>
			<p class="text-muted-foreground">
				Shadcn-svelte component registry for BlossomOS. Colors compile from
				<code class="bg-muted rounded px-1 py-0.5 text-xs">colors.json</code>.
			</p>
		</div>
	</header>

	<main class="flex flex-1 flex-col gap-8">
		<section class="flex flex-col gap-4 rounded-(--radius-card) border bg-card p-6">
			<h2 class="text-sm font-medium">Variants</h2>
			<div class="flex flex-wrap gap-3">
				{#each variants as variant (variant)}
					<Button {variant}>{variant}</Button>
				{/each}
			</div>
		</section>

		<section class="flex flex-col gap-4 rounded-(--radius-card) border bg-card p-6">
			<h2 class="text-sm font-medium">Sizes</h2>
			<div class="flex flex-wrap items-center gap-3">
				{#each sizes as size (size)}
					<Button {size}>{size === "icon" ? "+" : size}</Button>
				{/each}
			</div>
		</section>

		<section class="flex flex-col gap-4 rounded-(--radius-card) border bg-card p-6">
			<h2 class="text-sm font-medium">Mode Toggle</h2>
			<ModeToggle />
		</section>
	</main>
</div>
